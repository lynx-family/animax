// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/ios/loader_wrapper_ios.h"
#include "src/base/log/log.h"
#include "src/resource/resource_loader/ios/resource_util.h"

#import <AnimaX/AnimaXLoaderRequest.h>
#import <AnimaX/AnimaXLoaderResponse.h>
#import "AnimaXLoaderResponse+Internal.h"

namespace lynx {
namespace animax {

namespace {

NSString* MakeRequestURL(const ResourceRequest& request) {
  if (!request.uri_info.Valid()) {
    return nil;
  }
  DCHECK(!request.uri_info.uri.empty());
  return [NSString stringWithUTF8String:request.uri_info.uri.data()];
}

NSDictionary<NSString*, id>* MakeRequestProps(const ResourceRequest& request) {
  NSDictionary<NSString*, id>* request_props = [NSDictionary dictionary];
  if (request.uri_info.content_type == UriInfo::ContentType::kImage) {
    request_props = @{
      @"image_width" : @(request.width),
      @"image_height" : @(request.height),
    };
  }
  return request_props;
}

AnimaXLoaderRequest* ResourceRequestToAnimaXResourceRequest(const ResourceRequest& request) {
  AnimaXLoaderRequest* animax_request =
      [AnimaXLoaderRequest requestWithURL:MakeRequestURL(request)
                        requestProperties:MakeRequestProps(request)];
  return animax_request;
}

TransformRequestIOS AnimaXLoaderResponseToTransformRequestIOS(ResourceRequest request,
                                                              AnimaXLoaderResponse* response) {
  const auto request_type = request.type;
  auto transform_request = TransformRequestIOS{};
  transform_request.input = response.payload;

  switch (response.payloadType) {
    case AnimaXLoaderPayloadTypeNSData: {
      transform_request.input_type = TransformRequestInputType::kNSData;
      break;
    }
    case AnimaXLoaderPayloadTypeFilePathNSString: {
      transform_request.input_type = TransformRequestInputType::kFilePathNSString;
      break;
    }
    case AnimaXLoaderPayloadTypeInvalid:
      [[fallthrough]];
    default:
      break;
  }

  switch (request_type) {
    case ResourceRequestType::kLoadBitmap: {
      transform_request.output_type = ResourcePayloadType::kBitmap;
      break;
    }
    case ResourceRequestType::kLoadRawData: {
      transform_request.output_type = ResourcePayloadType::kRawData;
      break;
    }
    case ResourceRequestType::kDownloadToLocal: {
      transform_request.output_type = ResourcePayloadType::kFilePath;
      transform_request.default_file_path_for_ns_data_file_path_conversion =
          GetTempFilePathForUrl(request.uri_info.uri);
      break;
    }
    default:
      break;
  }

  return transform_request;
}

LoaderError LoaderErrorFromNSError(NSError* error) {
  auto* error_c_str = [[error description] UTF8String];
  return LoaderError{.code = kUserDefined,
                     .message = error_c_str ? std::string{error_c_str} : std::string{}};
}
}  // namespace

void LoaderWrapperIOS::Load(ResourceRequest request, CallbackType callback) {
  if (nil == ios_loader_) {
    callback(TransformRequestIOS{},
             LoaderError{.code = kInvalidLoader, .message = "AnimaXLoaderProtocol is nil."});
    return;
  }
  if (request.type == ResourceRequestType::kInvalid) {
    callback(TransformRequestIOS{},
             LoaderError{.code = kInvalidArgument, .message = "Invalid Request Type."});
    return;
  }
  AnimaXLoaderRequest* animax_request = ResourceRequestToAnimaXResourceRequest(request);
  if (animax_request.url == nil || [animax_request.url length] == 0) {
    callback(TransformRequestIOS{},
             LoaderError{.code = kInvalidArgument, .message = "Invalid URI."});
    return;
  }
  auto shared_callback = std::shared_ptr<CallbackType>(new CallbackType{std::move(callback)});
  [ios_loader_ handleRequest:animax_request
                  completion:^(AnimaXLoaderResponse* _Nonnull response) {
                    auto error = LoaderError{};
                    auto result = TransformRequestIOS{};

                    if (nil != response.error) {
                      error = LoaderErrorFromNSError(response.error);
                    } else if (nil == response.payload) {
                      error = LoaderError{.code = kInvalidArgument,
                                          .message = "AnimaXLoaderResponse payload is nil."};
                    } else {
                      result = AnimaXLoaderResponseToTransformRequestIOS(request, response);
                    }

                    auto& cb = *shared_callback;
                    cb(std::move(result), std::move(error));
                  }];
}

}  // namespace animax
}  // namespace lynx
