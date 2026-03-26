// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/ios/base_transform_loaders_ios.h"
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXImageDecoderService.h>
#import "AnimaXContext+Internal.h"
#include "src/base/log/log.h"
#include "src/resource/resource_loader/ios/resource_util.h"

namespace lynx {
namespace animax {

NSDataBitmapTransformLoader::NSDataBitmapTransformLoader(AnimaXContext* context) {
  if (context) {
    decoder_impl_ = [context.ability getService:@protocol(AnimaXImageDecoderService)];
  }
}

void NSDataBitmapTransformLoader::Load(TransformRequestIOS request, CallbackType callback) {
  auto response = TransformResponseIOS{};
  auto bitmap = NSDataToBitmap(request.input);

  if (!bitmap && decoder_impl_) {
    UIImage* ui_image = [decoder_impl_ loadImageData:request.input];
    if (ui_image) {
      bitmap = UIImageToBitmap(ui_image);
    } else {
      ANIMAX_LOGE("Failed to decode image by AnimaXImageDecoderService.");
    }
  }

  if (!bitmap) {
    callback(ResourceResponse{}, LoaderError{.code = kInvalidImageData, "Failed to decode image."});
    return;
  }
  response.payload = MakeBitmapResourcePayload(std::move(bitmap));
  callback(std::move(response), LoaderError{});
}

void NSDataRawDataTransformLoader::Load(TransformRequestIOS request, CallbackType callback) {
  auto response = TransformResponseIOS{};
  auto raw_data = NSDataToRawData(request.input);
  if (!raw_data) {
    callback(ResourceResponse{},
             LoaderError{.code = kInvalidImageData, "Failed to convert NSData to RawData."});
    return;
  }
  response.payload = MakeRawDataResourcePayload(std::move(raw_data));
  callback(std::move(response), LoaderError{});
}

void FilePathNSStringToFilePathStringLoader::Load(TransformRequestIOS request,
                                                  CallbackType callback) {
  NSString* file_path = request.input;
  auto response = TransformResponseIOS{};
  if (file_path == nil) {
    response.payload.path = std::string{};
    callback(std::move(response), LoaderError{.code = kInvalidArgument, "File Path is empty."});
    return;
  }
  response.payload = MakePathResourcePayload(std::string{[file_path UTF8String]});
  callback(std::move(response), LoaderError{});
}

void FilePathNSStringToNSDataLoader::Load(TransformRequestIOS request, CallbackType callback) {
  NSString* file_path = request.input;
  NSFileManager* manager = [NSFileManager defaultManager];
  if (file_path == nil) {
    callback(std::move(request), LoaderError{.code = kInvalidArgument, "File Path is empty."});
    return;
  }
  if (![manager fileExistsAtPath:file_path]) {
    callback(std::move(request), LoaderError{.code = kFileDoesNotExist, "File does not exist."});
    return;
  }

  NSData* file_content = [NSData dataWithContentsOfFile:file_path];
  auto new_request = TransformRequestIOS{.input = file_content,
                                         .input_type = TransformRequestInputType::kNSData,
                                         .output_type = request.output_type};
  callback(std::move(new_request), LoaderError{});
}

void NSDataToFilePathStringLoader::Load(TransformRequestIOS request, CallbackType callback) {
  auto file_path = std::string{};

  if (request.default_file_path_for_ns_data_file_path_conversion.empty()) {
    NSString* ns_data_hash = [[NSNumber numberWithUnsignedLong:[request.input hash]] stringValue];
    auto hash_str = std::string{[ns_data_hash UTF8String]};
    file_path = GetTempFilePathForUrl(hash_str);
  } else {
    file_path = request.default_file_path_for_ns_data_file_path_conversion;
  }

  if (!WriteNSDataToFile(request.input, file_path)) {
    callback({}, LoaderError{.code = kFileNoAccess, "Unable to write data to file: " + file_path});
    return;
  }
  callback(ResourceResponse{.payload = MakePathResourcePayload(file_path)}, {});
}

}  // namespace animax
}  // namespace lynx
