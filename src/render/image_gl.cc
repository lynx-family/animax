// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/image_gl.h"

#include "include/resource/bitmap.h"
#include "skity/graphic/image.hpp"
#include "src/base/thread/thread_assert.h"
#include "src/render/image_gl_factory.h"
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

  image_ = CreateGLPixmapImage(pixmap, real_context);
}

ImageGL::ImageGL(TextureInfo *texture, RealContext *real_context) {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  image_ =
      CreateGLTextureImage(static_cast<TextureInfoGL *>(texture), real_context);
}

}  // namespace animax
}  // namespace lynx
