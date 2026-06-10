// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/image_mtl_factory.h"

#include "skity/graphic/image.hpp"
#include "src/render/skity_promise_texture.h"
#include "src/render/texture_info_mtl.h"

namespace lynx {
namespace animax {

namespace {

using MTLTexturePromiseTexture = SkityPromiseTexture<id<MTLTexture>>;

}  // namespace

std::shared_ptr<skity::Image> CreateMetalTextureImage(TextureInfoMTL *texture_info,
                                                      RealContext *real_context) {
  if (!texture_info) {
    return {};
  }

  id<MTLTexture> mtl_texture = texture_info->GetMTLTexture();
  if (!mtl_texture) {
    return {};
  }

  auto promise = new MTLTexturePromiseTexture(mtl_texture, MakeTextureFromMetalTexture);
  return skity::Image::MakePromiseTextureImage2(
      skity::TextureFormat::kRGBA, mtl_texture.width, mtl_texture.height,
      skity::AlphaType::kPremul_AlphaType, &MTLTexturePromiseTexture::GeneratePromiseTexture,
      &MTLTexturePromiseTexture::ReleasePromiseContext, promise);
}

}  // namespace animax
}  // namespace lynx
