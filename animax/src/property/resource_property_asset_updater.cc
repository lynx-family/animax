// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/resource_property_asset_updater.h"

#include "src/property/property_update_util.h"
#include "src/resource/asset/audio_asset.h"
#include "src/resource/asset/font_asset.h"
#include "src/resource/asset/image_asset.h"
#include "src/resource/asset/video_asset.h"

namespace lynx {
namespace animax {
void ResourcePropertyAssetUpdator::Visit(FontAsset& asset) {
  auto asset_model = asset.Model();
  bool success = true;
  switch (context_.GetResourceType()) {
    case ResourcePropertyType::kFontFamily: {
      success = context_.GetValue().CopyTo(&asset_model.family);
      break;
    }
    case ResourcePropertyType::kFontStyle: {
      success = context_.GetValue().CopyTo(&asset_model.style);
      break;
    }
    case ResourcePropertyType::kFontPath: {
      success = context_.GetValue().CopyTo(&asset_model.path);
      break;
    }
    case ResourcePropertyType::kFontAscent: {
      success = context_.GetValue().CopyTo(&asset_model.ascent);
      break;
    }
    default: {
      context_.SetErrorType(PropertyUpdateResult::kPropertyNotImplemented);
      break;
    }
  }
  if (!success) {
    context_.SetErrorType(PropertyUpdateResult::kValueInvalid);
  }
  asset.ResetModel(asset_model);
}

void ResourcePropertyAssetUpdator::Visit(ImageAsset& asset) {
  auto asset_model = asset.Model();
  bool success = true;
  switch (context_.GetResourceType()) {
    case ResourcePropertyType::kImageDirName: {
      success = context_.GetValue().CopyTo(&asset_model.dir_name);
      break;
    }
    case ResourcePropertyType::kImageFileName: {
      success = context_.GetValue().CopyTo(&asset_model.file_name);
      break;
    }
    case ResourcePropertyType::kImageWidth: {
      success = context_.GetValue().CopyTo(&asset_model.width);
      break;
    }
    case ResourcePropertyType::kImageHeight: {
      success = context_.GetValue().CopyTo(&asset_model.height);
      break;
    }
    default: {
      context_.SetErrorType(PropertyUpdateResult::kPropertyNotImplemented);
      break;
    }
  }
  if (!success) {
    context_.SetErrorType(PropertyUpdateResult::kValueInvalid);
  }
  asset.ResetModel(asset_model);
}

void ResourcePropertyAssetUpdator::Visit(VideoAsset& asset) {
  auto asset_model = asset.Model();
  bool success = true;
  switch (context_.GetResourceType()) {
    case ResourcePropertyType::kVideoDirName: {
      success = context_.GetValue().CopyTo(&asset_model.dir_name);
      break;
    }
    case ResourcePropertyType::kVideoFileName: {
      success = context_.GetValue().CopyTo(&asset_model.file_name);
      break;
    }
    case ResourcePropertyType::kVideoWidth: {
      // TODO(aiyongbiao.rick) Modify the Rendering Size of the Video
      break;
    }
    case ResourcePropertyType::kVideoHeight: {
      // TODO(aiyongbiao.rick) Modify the Rendering Size of the Video
      break;
    }
    default: {
      context_.SetErrorType(PropertyUpdateResult::kPropertyNotImplemented);
      break;
    }
  }
  if (!success) {
    context_.SetErrorType(PropertyUpdateResult::kValueInvalid);
  }
  asset.ResetModel(asset_model);
}

void ResourcePropertyAssetUpdator::Visit(AudioAsset& asset) {
  auto asset_model = asset.Model();
  bool success = true;
  switch (context_.GetResourceType()) {
    case ResourcePropertyType::kAudioDirName: {
      success = context_.GetValue().CopyTo(&asset_model.dir_name);
      break;
    }
    case ResourcePropertyType::kAudioFileName: {
      success = context_.GetValue().CopyTo(&asset_model.file_name);
      break;
    }
    default: {
      context_.SetErrorType(PropertyUpdateResult::kPropertyNotImplemented);
      break;
    }
  }
  if (!success) {
    context_.SetErrorType(PropertyUpdateResult::kValueInvalid);
  }
  asset.ResetModel(asset_model);
}
}  // namespace animax
}  // namespace lynx
