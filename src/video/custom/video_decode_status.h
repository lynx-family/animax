// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_DECODE_STATUS_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_DECODE_STATUS_H_

#include <cstdint>

namespace lynx {
namespace animax {

enum class VideoSendStatus : int32_t {
  kSuccess,
  kError,
};

enum class VideoDecodeStatus : int32_t {
  kFrameReady,
  kNeedInput,
  kEndOfStream,
  kError,
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_DECODE_STATUS_H_
