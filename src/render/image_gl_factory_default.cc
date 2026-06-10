// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "skity/graphic/image.hpp"
#include "src/render/image_gl_factory.h"
#include "src/render/real_context.h"
#include "src/render/texture_info_gl.h"

namespace lynx {
namespace animax {

std::shared_ptr<skity::Image> CreateGLPixmapImage(
    std::shared_ptr<skity::Pixmap> pixmap, RealContext *real_context) {
  if (!pixmap) {
    return {};
  }
  skity::GPUContext *gpu_context = real_context ? real_context->Get() : nullptr;
  return skity::Image::MakeImage(pixmap, gpu_context);
}

std::shared_ptr<skity::Image> CreateGLTextureImage(TextureInfoGL *texture_info,
                                                   RealContext *real_context) {
  if (!texture_info || !real_context) {
    return {};
  }
  return skity::Image::MakeHWImage(
      MakeTextureFromGLTexture(real_context->Get(), *texture_info));
}

}  // namespace animax
}  // namespace lynx
