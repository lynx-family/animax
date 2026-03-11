// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_TEXTURE_INFO_MULTI_GL_H_
#define ANIMAX_SRC_RENDER_TEXTURE_INFO_MULTI_GL_H_

#include "src/render/texture_info.h"

namespace lynx {
namespace animax {

class TextureInfoMultiGL : public TextureInfo {
 public:
  TextureInfoMultiGL(std::vector<uint32_t> textures, uint32_t width,
                     uint32_t height, uint32_t target)
      : TextureInfo(ContextBackend::kOpenGL, width, height),
        textures_(std::move(textures)),
        target_(target) {}

  const std::vector<uint32_t>& GetTextures() const { return textures_; }
  uint32_t Target() const { return target_; }

 private:
  std::vector<uint32_t> textures_;
  uint32_t target_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_TEXTURE_INFO_MULTI_GL_H_
