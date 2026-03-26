// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/buffer_copy_helper.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

bool BufferCopyHelper::IsSizeMatch(int32_t src_width, int32_t src_height,
                                   int32_t dst_width, int32_t dst_height) {
  if (dst_height != src_height || dst_width != src_width) {
    ANIMAX_LOGI("Buffer dimensions mismatch")
    return false;
  }
  return true;
}

void BufferCopyHelper::CopyBuffer(uint8_t *src, uint8_t *dst, int32_t height,
                                  int32_t src_stride_byte,
                                  int32_t dst_stride_byte) {
  if (!dst || !src || height <= 0 || src_stride_byte > dst_stride_byte) {
    ANIMAX_LOGI("Invalid input parameters")
    return;
  }
  for (int32_t i = 0; i < height; i++) {
    std::memcpy(dst, src, src_stride_byte);
    dst += dst_stride_byte;
    src += src_stride_byte;
  }
}

}  // namespace animax
}  // namespace lynx
