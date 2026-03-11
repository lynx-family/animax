// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/image_mtl.h"
#include "skity/gpu/gpu_context_mtl.h"
#include "skity/graphic/image.hpp"
#include "src/render/real_context.h"
#include "src/render/texture_info_mtl.h"

namespace lynx {
namespace animax {

ImageMTL::ImageMTL(TextureInfo *texture, RealContext *real_context) {
  auto context = real_context->Get();

  id<MTLTexture> mtl_texture = static_cast<TextureInfoMTL *>(texture)->GetMTLTexture();

  skity::GPUBackendTextureInfoMTL texture_info{};
  texture_info.backend = skity::GPUBackendType::kMetal;
  texture_info.format = skity::TextureFormat::kRGBA;
  texture_info.width = mtl_texture.width;
  texture_info.height = mtl_texture.height;
  texture_info.alpha_type = skity::AlphaType::kPremul_AlphaType;
  texture_info.texture = mtl_texture;

  image_ = skity::Image::MakeHWImage(context->WrapTexture(&texture_info));
}

}  // namespace animax
}  // namespace lynx
