// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/asset/image_asset.h"

#include "src/base/monitor/trace_event.h"
#include "src/base/thread/task_runner.h"
#include "src/render/canvas.h"
#include "src/render/image_gl.h"

namespace lynx {
namespace animax {

ImageAsset::ImageAsset(ImageAssetModel info) : model_{std::move(info)} {}

void ImageAsset::SetBitmap(std::unique_ptr<Bitmap> bitmap) {
  bitmap_ = std::move(bitmap);
}

bool ImageAsset::IsPolyfill() const { return Model().file_name == "%s"; }

bool ImageAsset::IsBase64() const {
  return Model().file_name.rfind("data:", 0) == 0 &&
         Model().file_name.find("base64,") != std::string::npos;
}

bool ImageAsset::IsLoaded() const { return bitmap_ || image_; }

Image* ImageAsset::GetImage(RealContext* real_context) {
  if (image_) {
    return image_.get();
  }

  if (bitmap_) {
    ANIMAX_TRACE_EVENT_BEGIN_ASSET_ID(kPrepareImageTexture, Model().id);
    image_ = std::make_unique<ImageGL>(std::move(bitmap_), real_context);
    ANIMAX_TRACE_EVENT_END();
    bitmap_ = nullptr;
    // file_name_ may hold large base64 data.
    // Clean up file_name_ after the bitmap is loaded.
    model_.file_name = std::string{};
  }

  return image_.get();
}

void ImageAsset::ResetModel(ImageAssetModel info) {
  model_ = std::move(info);
  bitmap_ = nullptr;
  image_ = nullptr;
}

}  // namespace animax
}  // namespace lynx
