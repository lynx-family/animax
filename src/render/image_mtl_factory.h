// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_IMAGE_MTL_FACTORY_H_
#define ANIMAX_SRC_RENDER_IMAGE_MTL_FACTORY_H_

#import <Metal/Metal.h>

#include <memory>

#include "skity/gpu/gpu_context_mtl.h"

namespace skity {
class Image;
class Texture;
}  // namespace skity

namespace lynx {
namespace animax {

class RealContext;
class TextureInfoMTL;

inline std::shared_ptr<skity::Texture> MakeTextureFromMetalTexture(
    skity::GPUContext *context, id<MTLTexture> texture) {
  if (!context || !texture) {
    return {};
  }

  skity::GPUBackendTextureInfoMTL texture_info{};
  texture_info.backend = skity::GPUBackendType::kMetal;
  texture_info.format = skity::TextureFormat::kRGBA;
  texture_info.width = texture.width;
  texture_info.height = texture.height;
  texture_info.alpha_type = skity::AlphaType::kPremul_AlphaType;
  texture_info.texture = texture;

  return context->WrapTexture(&texture_info);
}

std::shared_ptr<skity::Image> CreateMetalTextureImage(
    TextureInfoMTL *texture_info, RealContext *real_context);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_IMAGE_MTL_FACTORY_H_
