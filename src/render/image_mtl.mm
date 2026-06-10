// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/image_mtl.h"

#include "src/render/image_mtl_factory.h"
#include "src/render/texture_info_mtl.h"

namespace lynx {
namespace animax {

ImageMTL::ImageMTL(TextureInfo *texture, RealContext *real_context) {
  if (!texture) {
    return;
  }
  image_ = CreateMetalTextureImage(static_cast<TextureInfoMTL *>(texture), real_context);
}

}  // namespace animax
}  // namespace lynx
