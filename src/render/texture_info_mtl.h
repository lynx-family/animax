// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_TEXTURE_INFO_MTL_H_
#define ANIMAX_SRC_RENDER_TEXTURE_INFO_MTL_H_

#import <Metal/Metal.h>

#include "src/render/texture_info.h"

namespace lynx {
namespace animax {

class TextureInfoMTL : public TextureInfo {
 public:
  TextureInfoMTL(id<MTLTexture> texture, uint32_t width, uint32_t height)
      : TextureInfo(ContextBackend::kMetal, width, height),
        mtl_texture_(texture) {}

  id<MTLTexture> GetMTLTexture() const { return mtl_texture_; }

 private:
  id<MTLTexture> mtl_texture_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_TEXTURE_INFO_MTL_H_
