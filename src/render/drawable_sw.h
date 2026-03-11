// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_DRAWABLE_SW_H_
#define ANIMAX_SRC_RENDER_DRAWABLE_SW_H_

#include "src/render/drawable.h"

namespace lynx {
namespace animax {

class DrawableSW : public Drawable {
 public:
  DrawableSW(int32_t width, int32_t height, void* buffer, int32_t bytes_per_row)
      : Drawable(ContextBackend::kSoftware, width, height),
        buffer_(buffer),
        bytes_per_row_(bytes_per_row) {}

  void* GetBuffer() const { return buffer_; }

  int32_t GetBytesPerRow() const { return bytes_per_row_; }

 private:
  void* buffer_;
  int32_t bytes_per_row_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_DRAWABLE_SW_H_
