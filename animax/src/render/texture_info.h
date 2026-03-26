// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_TEXTURE_INFO_H_
#define ANIMAX_SRC_RENDER_TEXTURE_INFO_H_

#include "src/render/real_context.h"

namespace lynx {
namespace animax {

class TextureInfo {
 public:
  TextureInfo(ContextBackend backend, uint32_t width, uint32_t height)
      : backend_(backend), width_(width), height_(height) {}

  virtual ~TextureInfo() = default;

  ContextBackend GetBackendType() const { return backend_; }

  uint32_t Width() const { return width_; }
  uint32_t Height() const { return height_; }

 private:
  ContextBackend backend_;
  uint32_t width_;
  uint32_t height_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_TEXTURE_INFO_H_
