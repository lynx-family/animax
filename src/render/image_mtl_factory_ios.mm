// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/image_mtl_factory.h"

#include "skity/graphic/image.hpp"
#include "src/render/real_context.h"
#include "src/render/texture_info_mtl.h"

namespace lynx {
namespace animax {

std::shared_ptr<skity::Image> CreateMetalTextureImage(TextureInfoMTL *texture_info,
                                                      RealContext *real_context) {
  if (!texture_info || !real_context) {
    return {};
  }

  if (!real_context || real_context->GetBackendType() != ContextBackend::kMetal) {
    return {};
  }

  auto context = real_context->Get();

  return skity::Image::MakeHWImage(
      MakeTextureFromMetalTexture(context, texture_info->GetMTLTexture()));
}

}  // namespace animax
}  // namespace lynx
