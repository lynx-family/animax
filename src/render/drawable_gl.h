// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_DRAWABLE_GL_H_
#define ANIMAX_SRC_RENDER_DRAWABLE_GL_H_

#include "src/render/drawable.h"

namespace lynx {
namespace animax {
class DrawableGL : public Drawable {
 public:
  DrawableGL(uint32_t framebuffer, int32_t width, int32_t height,
             bool enable_anti_aliasing)
      : Drawable{ContextBackend::kOpenGL, width, height},
        framebuffer_{framebuffer},
        enable_anti_aliasing_(enable_anti_aliasing) {}

  uint32_t Framebuffer() const { return framebuffer_; }
  bool EnableAntiAliasing() const { return enable_anti_aliasing_; }

 private:
  uint32_t framebuffer_;
  bool enable_anti_aliasing_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_DRAWABLE_GL_H_
