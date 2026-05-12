// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_HARMONY_VIDEO_CODEC_MANAGER_HARMONY_H_
#define ANIMAX_SRC_VIDEO_HARMONY_VIDEO_CODEC_MANAGER_HARMONY_H_

#include <multimedia/player_framework/native_avcodec_videodecoder.h>
#include <native_window/external_window.h>

#include <chrono>
#include <cstdint>
#include <memory>

namespace lynx {
namespace animax {

struct VideoData;

class VideoCodecManagerHarmony {
 public:
  VideoCodecManagerHarmony(const VideoCodecManagerHarmony &) = delete;
  VideoCodecManagerHarmony &operator=(const VideoCodecManagerHarmony &) =
      delete;

  virtual ~VideoCodecManagerHarmony();

  virtual bool Init() = 0;
  virtual bool RenderFrame(int32_t frame) = 0;

 protected:
  static constexpr const std::chrono::milliseconds kBaseTimeout{16};
  static constexpr const int32_t kMaxTimeoutCount = 5;
  static constexpr const int32_t kInvalidFrame = -1;

  VideoCodecManagerHarmony(VideoData *data, OHNativeWindow *native_window);

  bool InitCodec();
  void ReleaseCodec();

  int32_t GetPrevKeyframe(int32_t frame) const;
  int32_t AdvanceFrameCounter(int32_t frame) const;
  std::chrono::milliseconds GetTimeout(int32_t frame) const;
  void IncreaseTimeoutCount();
  bool IsValidFrame(int32_t frame) const;

  virtual bool SetupCodec() = 0;

  VideoData *data_ = nullptr;
  OHNativeWindow *native_window_ = nullptr;
  OH_AVCodec *av_codec_ = nullptr;

  int32_t timeout_count_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_HARMONY_VIDEO_CODEC_MANAGER_HARMONY_H_
