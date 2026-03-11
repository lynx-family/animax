// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_BUFFER_COPY_HELPER_H_
#define ANIMAX_SRC_BASE_UTIL_BUFFER_COPY_HELPER_H_

#include <stdint.h>

namespace lynx {
namespace animax {
class BufferCopyHelper {
 public:
  static bool IsSizeMatch(int32_t src_width, int32_t src_height,
                          int32_t dst_width, int32_t dst_height);
  static void CopyBuffer(uint8_t* src, uint8_t* dst, int32_t height,
                         int32_t src_stride_byte, int32_t dst_stride_byte);
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_BUFFER_COPY_HELPER_H_
