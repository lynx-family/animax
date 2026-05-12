// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/harmony/video_codec_manager_harmony.h"

#include <algorithm>

#include "src/base/log/log.h"
#include "src/video/harmony/video_asset_harmony.h"

namespace lynx {
namespace animax {

VideoCodecManagerHarmony::VideoCodecManagerHarmony(
    VideoData *data, OHNativeWindow *native_window)
    : data_(data), native_window_(native_window) {}

VideoCodecManagerHarmony::~VideoCodecManagerHarmony() { ReleaseCodec(); }

bool VideoCodecManagerHarmony::InitCodec() {
  if (data_ == nullptr || data_->video_format == nullptr ||
      data_->frame_count <= 0 || native_window_ == nullptr ||
      av_codec_ != nullptr) {
    ANIMAX_LOGE("InitCodec fail.");
    return false;
  }

  auto *video_format = data_->video_format.get();
  const char *mime_name = nullptr;
  OH_AVFormat_GetStringValue(video_format, OH_MD_KEY_CODEC_MIME, &mime_name);
  ANIMAX_LOGI("Create codec by mime:" << (mime_name ? mime_name : ""));

  av_codec_ = OH_VideoDecoder_CreateByMime(mime_name);
  if (av_codec_ == nullptr) {
    ANIMAX_LOGE("OH_VideoDecoder_CreateByMime fail");
    return false;
  }

  if (!SetupCodec()) {
    ReleaseCodec();
    return false;
  }

  OH_AVErrCode err_code = OH_VideoDecoder_SetSurface(av_codec_, native_window_);
  if (err_code != OH_AVErrCode::AV_ERR_OK) {
    ANIMAX_LOGE("OH_VideoDecoder_SetSurface fail");
    ReleaseCodec();
    return false;
  }
  err_code = OH_VideoDecoder_Prepare(av_codec_);
  if (err_code != OH_AVErrCode::AV_ERR_OK) {
    ANIMAX_LOGE("OH_VideoDecoder_Prepare fail");
    ReleaseCodec();
    return false;
  }

  err_code = OH_VideoDecoder_Start(av_codec_);
  if (err_code != OH_AVErrCode::AV_ERR_OK) {
    ANIMAX_LOGE("OH_VideoDecoder_Start fail");
    ReleaseCodec();
    return false;
  }
  ANIMAX_LOGI("InitCodec success.");
  return true;
}

void VideoCodecManagerHarmony::ReleaseCodec() {
  if (av_codec_) {
    OH_VideoDecoder_Flush(av_codec_);
    OH_VideoDecoder_SetSurface(av_codec_, nullptr);
    OH_VideoDecoder_Stop(av_codec_);
    OH_VideoDecoder_Destroy(av_codec_);
    av_codec_ = nullptr;
  }
}

int32_t VideoCodecManagerHarmony::GetPrevKeyframe(int32_t frame) const {
  auto &keyframe_index = data_->keyframe_index;
  if (keyframe_index.empty() || frame < keyframe_index.front()) {
    return 0;
  }

  auto it =
      std::lower_bound(keyframe_index.begin(), keyframe_index.end(), frame);

  if (it != keyframe_index.end() && *it == frame) {
    return *it;
  }

  if (it == keyframe_index.begin()) {
    return 0;
  }

  --it;
  return *it;
}

int32_t VideoCodecManagerHarmony::AdvanceFrameCounter(int32_t frame) const {
  if (data_ == nullptr || data_->frame_count <= 0) {
    return 0;
  }
  return (frame + 1) % data_->frame_count;
}

std::chrono::milliseconds VideoCodecManagerHarmony::GetTimeout(
    int32_t frame) const {
  // Default behavior: use longer timeout for keyframes.
  auto is_keyframe =
      std::find(data_->keyframe_index.begin(), data_->keyframe_index.end(),
                frame) != data_->keyframe_index.end();
  if (is_keyframe) {
    return kBaseTimeout * kMaxTimeoutCount;
  } else if (timeout_count_ > 0) {
    return kBaseTimeout * timeout_count_;
  } else {
    return kBaseTimeout;
  }
}

void VideoCodecManagerHarmony::IncreaseTimeoutCount() {
  if (timeout_count_ < kMaxTimeoutCount) {
    timeout_count_ += 1;
  }
}

bool VideoCodecManagerHarmony::IsValidFrame(int32_t frame) const {
  return data_ != nullptr && frame >= 0 && frame < data_->frame_count;
}

}  // namespace animax
}  // namespace lynx
