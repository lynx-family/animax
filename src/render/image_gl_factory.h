// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_IMAGE_GL_FACTORY_H_
#define ANIMAX_SRC_RENDER_IMAGE_GL_FACTORY_H_

#include <memory>

#include "skity/gpu/gpu_context_gl.hpp"
#include "skity/graphic/image.hpp"
#include "src/render/texture_info_gl.h"

namespace skity {
class Image;
class Pixmap;
class Texture;
}  // namespace skity

namespace lynx {
namespace animax {

class RealContext;

inline std::shared_ptr<skity::Texture> MakeTextureFromGLTexture(
    skity::GPUContext *context, TextureInfoGL texture) {
  if (!context) {
    return {};
  }

  skity::GPUBackendTextureInfoGL texture_info;
  texture_info.backend = skity::GPUBackendType::kOpenGL;
  texture_info.format = skity::TextureFormat::kRGBA;
  texture_info.width = texture.Width();
  texture_info.height = texture.Height();
  texture_info.alpha_type = skity::AlphaType::kPremul_AlphaType;
  texture_info.tex_id = texture.ID();
  texture_info.owned_by_engine = false;

  return context->WrapTexture(&texture_info);
}

std::shared_ptr<skity::Image> CreateGLPixmapImage(
    std::shared_ptr<skity::Pixmap> pixmap, RealContext *real_context);

std::shared_ptr<skity::Image> CreateGLTextureImage(TextureInfoGL *texture_info,
                                                   RealContext *real_context);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_IMAGE_GL_FACTORY_H_
