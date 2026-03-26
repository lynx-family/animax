// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_TEXTURE_INFO_GL_H_
#define ANIMAX_SRC_RENDER_TEXTURE_INFO_GL_H_

#include "src/render/texture_info.h"

namespace lynx {
namespace animax {

class TextureInfoGL : public TextureInfo {
 public:
  TextureInfoGL(uint32_t texture, uint32_t width, uint32_t height,
                uint32_t target)
      : TextureInfo(ContextBackend::kOpenGL, width, height),
        texture_(texture),
        target_(target) {}

  uint32_t ID() const { return texture_; }
  uint32_t Target() const { return target_; }

 private:
  uint32_t texture_;
  uint32_t target_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_TEXTURE_INFO_GL_H_
