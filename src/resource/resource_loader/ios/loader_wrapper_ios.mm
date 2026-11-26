// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/ios/loader_wrapper_ios.h"
#include <atomic>

#include "src/base/log/log.h"
#include "src/resource/resource_loader/ios/resource_util.h"

#import <AnimaX/AnimaXLoaderRequest.h>
#import <AnimaX/AnimaXLoaderResponse.h>
#import <AnimaX/AnimaXURLUtils.h>
#import <CommonCrypto/CommonDigest.h>
#import "AnimaXLoaderResponse+Internal.h"

namespace lynx {
namespace animax {

namespace {

NSString* ResourceCacheSource(id source) {
  static NSSet<NSString*>* sources;
  static dispatch_once_t once;
  dispatch_once(&once, ^{
    sources = [NSSet setWithArray:@[
      @"memory", @"disk", @"offline", @"http_cache", @"http_revalidated", @"cdn_cache", @"network",
      @"builtin", @"in_flight"
    ]];
  });
  return [source isKindOfClass:NSString.class] && [sources containsObject:source] ? source
                                                                                  : @"unknown";
}

NSString* ResourceURLHash(const void* bytes, NSUInteger length) {
  unsigned char digest[CC_SHA256_DIGEST_LENGTH];
  CC_SHA256(bytes, (CC_LONG)length, digest);
  return [[NSData dataWithBytes:digest length:sizeof(digest)] base64EncodedStringWithOptions:0];
}

NSString* ResourceTypeName(UriInfo::ContentType type) {
  switch (type) {
    case UriInfo::ContentType::kJson:
      return @"json";
    case UriInfo::ContentType::kZip:
      return @"zip";
    case UriInfo::ContentType::kFont:
      return @"font";
    case UriInfo::ContentType::kImage:
      return @"image";
    case UriInfo::ContentType::kVideo:
      return @"video";
    case UriInfo::ContentType::kAudio:
      return @"audio";
    default:
      return @"unknown";
  }
}

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
  transform_request.cache_by_content = request.cache_by_content;
  if (request.image_size_mode == ImageSizeMode::kExact) {
    transform_request.image_width = request.width;
    transform_request.image_height = request.height;
  }

  switch (response.payloadType) {
    case AnimaXLoaderPayloadTypeNSData: {
      transform_request.input_type = TransformRequestInputType::kNSData;
      break;
    }
    case AnimaXLoaderPayloadTypeFilePathNSString: {
      transform_request.input_type = TransformRequestInputType::kFilePathNSString;
      // Files unzipped from a host path are read after this request. Other
      // media is consumed right away and does not need a copy of the file.
      transform_request.cache_by_content &=
          request.uri_info.content_type == UriInfo::ContentType::kZip;
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
  id<AnimaXMonitorService> monitor = monitor_;
  NSString* request_id = nil;
  NSString* src_url = nil;
  if (monitor) {
    request_id = NSUUID.UUID.UUIDString;
    animax_request.resourceLoadInfo = [@{@"request_id" : request_id} mutableCopy];
    BOOL is_template = request.uri_info.content_type == UriInfo::ContentType::kJson ||
                       request.uri_info.content_type == UriInfo::ContentType::kZip;
    src_url = [AnimaXURLUtils
        clearUrlQuery:is_template ? animax_request.url : [monitor.urlHolder getCurrentUrl]];
  }
  // Keep only the per-request metadata alive while the host is loading.
  NSDictionary* load_info = animax_request.resourceLoadInfo;
  auto reported = monitor ? std::make_shared<std::atomic<bool>>(false) : nullptr;
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

                    const bool has_payload = !error;
                    const bool report = reported && !reported->exchange(true);
                    auto& cb = *shared_callback;
                    cb(std::move(result), std::move(error));
                    if (report) {
                      BOOL success = has_payload && [response.payload length] > 0;
                      BOOL cancelled = [load_info[@"cancelled"] isEqual:@YES] ||
                                       ([response.error.domain isEqualToString:NSURLErrorDomain] &&
                                        response.error.code == NSURLErrorCancelled);
                      [monitor reportResourceLoad:@{
                        @"request_id" : request_id,
                        @"phase" : @"consume",
                        @"resource_type" : ResourceTypeName(request.uri_info.content_type),
                        @"resource_url_hash" : ResourceURLHash(request.uri_info.uri.data(),
                                                               request.uri_info.uri.size()),
                        @"status" : success ? @"success" : (cancelled ? @"cancelled" : @"failed"),
                        @"cache_source" : success ? ResourceCacheSource(load_info[@"cache_source"])
                                                  : @"unknown",
                        AnimaXMonitorSourceUrl : src_url,
                      }];
                    }
                  }];
}

}  // namespace animax
}  // namespace lynx
