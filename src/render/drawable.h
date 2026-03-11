// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_DRAWABLE_H_
#define ANIMAX_SRC_RENDER_DRAWABLE_H_

#include "src/render/real_context.h"

namespace lynx {
namespace animax {
class Drawable {
 public:
  Drawable(ContextBackend backend, int32_t width, int32_t height)
      : backend_{backend}, width_{width}, height_{height} {}

  ContextBackend BackendType() const { return backend_; }
  int32_t Width() const { return width_; }
  int32_t Height() const { return height_; }

 private:
  ContextBackend backend_;
  int32_t width_;
  int32_t height_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_DRAWABLE_H_
