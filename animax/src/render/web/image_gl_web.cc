// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/resource/bitmap.h"
#include "skity/gpu/gpu_context_web.hpp"
#include "skity/graphic/image.hpp"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/render/canvas.h"
#include "src/render/image_gl.h"
#include "src/render/texture_info_gl.h"

namespace lynx {
namespace animax {
static inline std::shared_ptr<skity::Image> MakeImageFromTexture(
    std::unique_ptr<Bitmap> bitmap, RealContext *real_context) {
  DCHECK(real_context && bitmap && bitmap->HasTexture());
  WGPUTexture texture_web = reinterpret_cast<WGPUTexture>(bitmap->TextureID());
  skity::GPUBackendTextureInfoWEB texture_info;
  texture_info.backend = skity::GPUBackendType::kWebGPU;
  texture_info.width = wgpuTextureGetWidth(texture_web);
  texture_info.height = wgpuTextureGetHeight(texture_web);
  DCHECK(bitmap->Format() == BitmapFormat::kRGBA);
  texture_info.format = skity::TextureFormat::kRGBA;
  DCHECK(bitmap->AlphaType() == BitmapAlphaType::kUnpremul_AlphaType);
  texture_info.alpha_type = skity::AlphaType::kUnpremul_AlphaType;
  texture_info.texture = texture_web;
  auto skity_texture = real_context->Get()->WrapTexture(&texture_info);
  return skity::Image::MakeHWImage(std::move(skity_texture));
}

static inline std::shared_ptr<skity::Image> MakeImageFromPixels(
    std::unique_ptr<Bitmap> bitmap, RealContext *real_context) {
  DCHECK(real_context && bitmap && !bitmap->HasTexture());
  auto bitmap_ptr = bitmap.release();
  auto skity_raw_data = skity::Data::MakeWithProc(
      bitmap_ptr->Pixels(), bitmap_ptr->PixelsLength(),
      [](const void *ptr, void *context) {
        delete reinterpret_cast<Bitmap *>(context);
      },
      bitmap_ptr /*context*/);
  auto pixmap = std::make_shared<skity::Pixmap>(
      std::move(skity_raw_data), bitmap_ptr->BytesPerRow(), bitmap_ptr->Width(),
      bitmap_ptr->Height(),
      bitmap_ptr->AlphaType() == BitmapAlphaType::kPremul_AlphaType
          ? skity::AlphaType::kPremul_AlphaType
          : skity::AlphaType::kUnpremul_AlphaType);
  return skity::Image::MakeImage(pixmap, real_context->Get());
}

ImageGL::ImageGL(std::unique_ptr<Bitmap> bitmap, RealContext *real_context) {
  DCHECK(bitmap);
  if (bitmap->HasTexture()) {
    image_ = MakeImageFromTexture(std::move(bitmap), real_context);
  } else {
    image_ = MakeImageFromPixels(std::move(bitmap), real_context);
  }
}

ImageGL::ImageGL(TextureInfo *texture, RealContext *real_context) {
  // do not impl
}

}  // namespace animax
}  // namespace lynx
