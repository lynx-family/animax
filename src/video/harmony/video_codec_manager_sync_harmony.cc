// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <algorithm>
#include <cstring>

#include "src/base/log/log.h"
#include "src/base/util/harmony/video_decoder_sync_api_harmony.h"
#include "src/video/harmony/video_asset_harmony.h"
#include "src/video/harmony/video_codec_manager_factory_harmony.h"
#include "src/video/harmony/video_codec_manager_harmony.h"

namespace lynx {
namespace animax {
namespace {

constexpr const int32_t kMaxCacheFrameCount = 20;
constexpr const int32_t kMaxTryCount = 3;

int64_t GetTimeoutUs(std::chrono::milliseconds timeout) {
  return std::chrono::duration_cast<std::chrono::microseconds>(timeout).count();
}

class VideoCodecManagerSyncHarmony final : public VideoCodecManagerHarmony {
 public:
  VideoCodecManagerSyncHarmony(VideoData *data, OHNativeWindow *native_window)
      : VideoCodecManagerHarmony(data, native_window),
        sync_api_(HarmonyVideoDecoderSyncApi::GetInstance()) {}

  ~VideoCodecManagerSyncHarmony() override { ReleaseCodec(); }

  bool Init() override {
    ResetDecoderState();
    info_try_again_count_ = 0;
    UpdateCurrentMaxCacheFrameCount();
    bool success = InitCodec();
    if (success) {
      PrepareFrameToMaxCacheCapacity();
    }
    return success;
  }

  bool RenderFrame(int32_t frame) override {
    if (!IsValidFrame(frame) || av_codec_ == nullptr) {
      return false;
    }

    if (current_present_frame_ == frame) {
      if (max_cache_frame_count_ > 1) {
        PrepareFrameToMaxCacheCapacity();
      }
      return false;
    }

    RefreshPlayerState(frame);

    int32_t left_try_count = kMaxTryCount;
    bool rendered = false;
    while (current_present_frame_ != frame) {
      PrepareFrameToMaxCacheCapacity();
      if (cached_frame_ == 0) {
        if (left_try_count > 0) {
          --left_try_count;
          continue;
        }
        ANIMAX_LOGE("No cached video frame available");
        return false;
      }

      bool should_render = next_output_frame_ == frame;
      if (!ProcessOutputFrame(should_render)) {
        if (left_try_count <= 0) {
          return false;
        }
        --left_try_count;
        continue;
      }
      rendered = rendered || should_render;
      left_try_count = kMaxTryCount;
    }

    if (max_cache_frame_count_ > 1) {
      PrepareFrameToMaxCacheCapacity();
    }

    return rendered;
  }

 private:
  bool SetupCodec() override {
    auto *video_format = data_->video_format.get();
    if (!sync_api_.ConfigureSyncMode(video_format, true)) {
      ANIMAX_LOGE("Configure sync mode fail");
      return false;
    }

    OH_AVErrCode err_code = OH_VideoDecoder_Configure(av_codec_, video_format);
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGE("OH_VideoDecoder_Configure fail");
      return false;
    }
    return true;
  }

  OH_AVBuffer *AcquireInputBuffer(int32_t frame, uint32_t *index) {
    auto err_code = sync_api_.QueryInputBuffer(av_codec_, index,
                                               GetTimeoutUs(GetTimeout(frame)));
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      return nullptr;
    }

    OH_AVBuffer *buffer = sync_api_.GetInputBuffer(av_codec_, *index);
    if (buffer == nullptr) {
      ANIMAX_LOGE("OH_VideoDecoder_GetInputBuffer returned nullptr");
      return nullptr;
    }

    return buffer;
  }

  bool AcquireOutputBuffer(int32_t frame, uint32_t *index) {
    auto err_code = sync_api_.QueryOutputBuffer(
        av_codec_, index, GetTimeoutUs(GetTimeout(frame)));
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      return false;
    }

    OH_AVBuffer *buffer = sync_api_.GetOutputBuffer(av_codec_, *index);
    if (buffer == nullptr) {
      ANIMAX_LOGE("OH_VideoDecoder_GetOutputBuffer returned nullptr");
      return false;
    }

    return true;
  }

  bool QueueInputFrame(int32_t frame) {
    if (!IsValidFrame(frame) || cached_frame_ >= max_cache_frame_count_) {
      return false;
    }

    uint32_t input_index = 0;
    OH_AVBuffer *input_buffer = AcquireInputBuffer(frame, &input_index);
    if (input_buffer == nullptr) {
      return false;
    }

    auto &frame_info = data_->frame_list[frame];
    int32_t capacity = OH_AVBuffer_GetCapacity(input_buffer);
    if (capacity < frame_info.size) {
      ANIMAX_LOGE("Input buffer is too small, capacity: "
                  << capacity << ", frame size: " << frame_info.size);
      return false;
    }

    uint8_t *addr = OH_AVBuffer_GetAddr(input_buffer);
    if (addr == nullptr) {
      ANIMAX_LOGE("OH_AVBuffer_GetAddr returned nullptr");
      return false;
    }

    std::memcpy(addr, data_->buffer_data.data() + frame_info.begin,
                frame_info.size);

    OH_AVCodecBufferAttr attr{.pts = frame_info.timestamp,
                              .size = frame_info.size,
                              .offset = 0,
                              .flags = frame_info.flags};
    if (OH_AVBuffer_SetBufferAttr(input_buffer, &attr) !=
        OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGE("OH_AVBuffer_SetBufferAttr failed");
      return false;
    }

    auto err_code = OH_VideoDecoder_PushInputBuffer(av_codec_, input_index);
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGE("OH_VideoDecoder_PushInputBuffer failed");
      return false;
    }

    ++cached_frame_;
    if (next_output_frame_ == kInvalidFrame) {
      next_output_frame_ = next_input_frame_;
    }
    next_input_frame_ = AdvanceFrameCounter(next_input_frame_);
    return true;
  }

  bool PrepareNextFrame() { return QueueInputFrame(next_input_frame_); }

  void PrepareFrameToMaxCacheCapacity() {
    while (PrepareNextFrame()) {
    }
  }

  bool ProcessOutputFrame(bool render) {
    uint32_t output_index = 0;
    if (!AcquireOutputBuffer(next_output_frame_, &output_index)) {
      OnOutputTryAgain();
      return false;
    }

    OH_AVErrCode err_code;
    if (render) {
      err_code = OH_VideoDecoder_RenderOutputBuffer(av_codec_, output_index);
    } else {
      err_code = OH_VideoDecoder_FreeOutputBuffer(av_codec_, output_index);
    }

    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGE("Failed to process output buffer");
      return false;
    }

    if (cached_frame_ > 0) {
      --cached_frame_;
    }
    current_present_frame_ = next_output_frame_;
    next_output_frame_ = AdvanceFrameCounter(next_output_frame_);
    return true;
  }

  void RefreshPlayerState(int32_t frame) {
    int32_t key_frame = GetPrevKeyframe(frame);
    if (next_output_frame_ == kInvalidFrame) {
      cached_frame_ = 0;
      next_input_frame_ = key_frame;
      return;
    }

    int32_t frame_from_key_frame = frame - key_frame + 1;
    int32_t frame_from_current =
        (current_present_frame_ <= frame)
            ? (frame - current_present_frame_)
            : (frame + data_->frame_count - current_present_frame_);

    if (frame_from_current <= cached_frame_) {
      frame_from_current = 0;
    } else {
      frame_from_current -= cached_frame_;
    }

    if (frame_from_current > frame_from_key_frame) {
      if (max_cache_frame_count_ > 1) {
        if (!FlushDecoder()) {
          ANIMAX_LOGE("Failed to flush decoder");
        }
      }
      cached_frame_ = 0;
      next_input_frame_ = key_frame;
      next_output_frame_ = kInvalidFrame;
    }
  }

  bool FlushDecoder() {
    if (av_codec_ == nullptr) {
      return false;
    }

    OH_AVErrCode err_code = OH_VideoDecoder_Flush(av_codec_);
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGE("OH_VideoDecoder_Flush failed");
      return false;
    }

    err_code = OH_VideoDecoder_Start(av_codec_);
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGE("OH_VideoDecoder_Start after flush failed");
      return false;
    }

    return true;
  }

  void ResetDecoderState() {
    current_present_frame_ = kInvalidFrame;
    cached_frame_ = 0;
    next_input_frame_ = 0;
    next_output_frame_ = kInvalidFrame;
    timeout_count_ = 0;
  }

  void OnOutputTryAgain() {
    ++info_try_again_count_;
    UpdateCurrentMaxCacheFrameCount();
  }

  void UpdateCurrentMaxCacheFrameCount() {
    if (info_try_again_count_ < 0) {
      info_try_again_count_ = 0;
    }

    // Keep the cache growth policy aligned with Android CodecManager.
    if (info_try_again_count_ > 1) {
      max_cache_frame_count_ =
          std::min(kMaxCacheFrameCount, 5 + info_try_again_count_);
    } else {
      max_cache_frame_count_ =
          std::min(kMaxCacheFrameCount, 3 + 3 * info_try_again_count_);
    }
  }

  HarmonyVideoDecoderSyncApi &sync_api_;
  int32_t current_present_frame_ = kInvalidFrame;
  int32_t next_input_frame_ = 0;
  int32_t next_output_frame_ = kInvalidFrame;
  int32_t cached_frame_ = 0;
  int32_t max_cache_frame_count_ = 1;
  int32_t info_try_again_count_ = 0;
};

}  // namespace

std::shared_ptr<VideoCodecManagerHarmony> MakeVideoCodecManagerSyncHarmony(
    VideoData *data, OHNativeWindow *native_window) {
  if (!HarmonyVideoDecoderSyncApi::GetInstance().IsSupported()) {
    return nullptr;
  }

  return std::make_shared<VideoCodecManagerSyncHarmony>(data, native_window);
}

}  // namespace animax
}  // namespace lynx
