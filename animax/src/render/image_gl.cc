// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/image_gl.h"

#include "include/resource/bitmap.h"
#include "skity/gpu/gpu_context_gl.hpp"
#include "skity/graphic/image.hpp"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/render/canvas.h"
#include "src/render/texture_info_gl.h"

namespace lynx {
namespace animax {

ImageGL::ImageGL(std::unique_ptr<Bitmap> bitmap, RealContext *real_context) {
  auto bitmap_ptr = bitmap.release();
  auto skity_raw_data = skity::Data::MakeWithProc(
      bitmap_ptr->Pixels(), bitmap_ptr->PixelsLength(),
      [](const void *ptr, void *context) {
        delete reinterpret_cast<Bitmap *>(context);
      },
      bitmap_ptr /*context*/);
  auto pixmap = std::make_shared<skity::Pixmap>(
      skity_raw_data, bitmap_ptr->BytesPerRow(), bitmap_ptr->Width(),
      bitmap_ptr->Height(),
      bitmap_ptr->AlphaType() == BitmapAlphaType::kPremul_AlphaType
          ? skity::AlphaType::kPremul_AlphaType
          : skity::AlphaType::kUnpremul_AlphaType);

  skity::GPUContext *gpu_ctx = nullptr;
  if (real_context) {
    gpu_ctx = real_context->Get();
  }

  image_ = skity::Image::MakeImage(pixmap, gpu_ctx);
}

ImageGL::ImageGL(TextureInfo *texture, RealContext *real_context) {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  DCHECK(real_context);
  auto texture_gl = static_cast<TextureInfoGL *>(texture);
  auto context = real_context->Get();
  skity::GPUBackendTextureInfoGL texture_info;
  texture_info.backend = skity::GPUBackendType::kOpenGL;
  texture_info.format = skity::TextureFormat::kRGBA;
  texture_info.width = texture_gl->Width();
  texture_info.height = texture_gl->Height();
  texture_info.alpha_type = skity::AlphaType::kPremul_AlphaType;

  texture_info.tex_id = texture_gl->ID();
  texture_info.owned_by_engine = false;

  auto skity_texture = context->WrapTexture(&texture_info);

  image_ = skity::Image::MakeHWImage(skity_texture);
}

}  // namespace animax
}  // namespace lynx
