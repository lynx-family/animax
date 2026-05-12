// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <condition_variable>
#include <cstring>
#include <mutex>
#include <queue>

#include "src/base/log/log.h"
#include "src/base/util/harmony/video_decoder_sync_api_harmony.h"
#include "src/video/harmony/video_asset_harmony.h"
#include "src/video/harmony/video_codec_manager_factory_harmony.h"
#include "src/video/harmony/video_codec_manager_harmony.h"

namespace lynx {
namespace animax {
namespace {

struct CodecBufferInfo {
  uint32_t index = 0;
  OH_AVBuffer *buffer = nullptr;
  OH_AVCodecBufferAttr attr = {0, 0, 0, AVCODEC_BUFFER_FLAGS_NONE};

  CodecBufferInfo() = default;
  CodecBufferInfo(uint32_t arg_index, OH_AVBuffer *arg_buffer)
      : index(arg_index), buffer(arg_buffer) {
    OH_AVBuffer_GetBufferAttr(buffer, &attr);
  }

  uint8_t *GetAddr() { return OH_AVBuffer_GetAddr(buffer); }
};

struct CodecData {
  std::mutex in_mutex;
  std::condition_variable in_cond;
  std::queue<CodecBufferInfo> in_queue;

  std::mutex out_mutex;
  std::condition_variable out_cond;
  std::queue<CodecBufferInfo> out_queue;
};

class VideoCodecManagerAsyncHarmony final : public VideoCodecManagerHarmony {
 public:
  VideoCodecManagerAsyncHarmony(VideoData *data, OHNativeWindow *native_window)
      : VideoCodecManagerHarmony(data, native_window) {}

  ~VideoCodecManagerAsyncHarmony() override { ReleaseCodec(); }

  bool Init() override {
    next_output_frame_ = 0;
    next_input_frame_ = 0;
    timeout_count_ = 0;
    return InitCodec();
  }

  bool RenderFrame(int32_t frame) override {
    if (!IsValidFrame(frame) || av_codec_ == nullptr) {
      return false;
    }

    if (next_output_frame_ == AdvanceFrameCounter(frame)) {
      return false;
    }

    UpdateFrameCounters(frame);

    while (next_output_frame_ <= frame) {
      if (!DecodeFrame(next_input_frame_)) {
        ANIMAX_LOGE("Failed to decode frame: " << next_input_frame_);
        return false;
      }
      next_input_frame_ = AdvanceFrameCounter(next_input_frame_);

      if (!ProcessOutputFrame(next_output_frame_ == frame)) {
        ANIMAX_LOGE("Failed to process output frame: " << next_output_frame_);
        return false;
      }
      next_output_frame_ = AdvanceFrameCounter(next_output_frame_);

      // Play from the start, exit the process loop
      if (next_output_frame_ == 0) {
        break;
      }
    }

    return true;
  }

 private:
  bool SetupCodec() override {
    auto *video_format = data_->video_format.get();
    if (HarmonyVideoDecoderSyncApi::GetInstance().IsSupported()) {
      if (!HarmonyVideoDecoderSyncApi::GetInstance().ConfigureSyncMode(
              video_format, false)) {
        ANIMAX_LOGE("Configure async mode fail");
        return false;
      }
    }
    OH_AVErrCode err_code = OH_VideoDecoder_Configure(av_codec_, video_format);
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGE("OH_VideoDecoder_Configure fail");
      return false;
    }

    OH_AVCodecCallback cb = {&OnError, &OnStreamChanged, &OnNeedInputBuffer,
                             &OnNeedOutputBuffer};
    err_code = OH_VideoDecoder_RegisterCallback(av_codec_, cb, &codec_data_);
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGE("OH_VideoDecoder_RegisterCallback fail");
      return false;
    }
    return true;
  }

  void UpdateFrameCounters(int32_t frame) {
    auto prev_keyframe = GetPrevKeyframe(frame);
    auto steps_from_keyframe = frame - prev_keyframe;
    auto steps_from_current = frame - next_output_frame_;

    /**
     * There are 2 cases that need to flush the cache and reset the frame
     * counter:
     * 1. The target frame is larger than the current one, and the target frame
     * has a closer keyframe.
     * 2. The target frame is less than the current one. Reverse seeking has
     * poor performance for now.
     */
    if ((steps_from_current > 0 && steps_from_current > steps_from_keyframe) ||
        steps_from_current < 0) {
      next_input_frame_ = prev_keyframe;
      next_output_frame_ = prev_keyframe;
    }
  }

  bool DecodeFrame(int32_t frame) {
    auto &frame_info = data_->frame_list[frame];
    std::unique_lock<std::mutex> lock(codec_data_.in_mutex);
    if (!codec_data_.in_cond.wait_for(lock, GetTimeout(frame), [this]() {
          return !codec_data_.in_queue.empty();
        })) {
      IncreaseTimeoutCount();
      ANIMAX_LOGE("Timeout waiting for input buffer");
      return false;
    }

    CodecBufferInfo in_info = codec_data_.in_queue.front();
    codec_data_.in_queue.pop();
    lock.unlock();

    // Copy frame data to input buffer
    std::memcpy(in_info.GetAddr(), data_->buffer_data.data() + frame_info.begin,
                frame_info.size);

    // Set buffer attributes
    OH_AVCodecBufferAttr attr{.pts = frame_info.timestamp,
                              .size = frame_info.size,
                              .offset = 0,
                              .flags = frame_info.flags};
    OH_AVBuffer_SetBufferAttr(in_info.buffer, &attr);

    // Push input buffer to decoder
    auto err_code = OH_VideoDecoder_PushInputBuffer(av_codec_, in_info.index);
    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGE("OH_VideoDecoder_PushInputBuffer failed");
      return false;
    }

    return true;
  }

  bool ProcessOutputFrame(bool render) {
    std::unique_lock<std::mutex> out_lock(codec_data_.out_mutex);
    if (!codec_data_.out_cond.wait_for(
            out_lock, GetTimeout(next_output_frame_),
            [this]() { return !codec_data_.out_queue.empty(); })) {
      IncreaseTimeoutCount();
      ANIMAX_LOGE("Timeout waiting for output buffer");
      return false;
    }

    CodecBufferInfo out_info = codec_data_.out_queue.front();
    codec_data_.out_queue.pop();
    out_lock.unlock();

    OH_AVErrCode err_code;
    if (render) {
      err_code = OH_VideoDecoder_RenderOutputBuffer(av_codec_, out_info.index);
    } else {
      err_code = OH_VideoDecoder_FreeOutputBuffer(av_codec_, out_info.index);
    }

    if (err_code != OH_AVErrCode::AV_ERR_OK) {
      ANIMAX_LOGE("Failed to process output buffer");
      return false;
    }

    return true;
  }

  static void OnError(OH_AVCodec *codec, int32_t error_code, void *user_data) {
    ANIMAX_LOGI("OnError");
  }

  static void OnStreamChanged(OH_AVCodec *codec, OH_AVFormat *format,
                              void *user_data) {
    // ignore
  }

  static void OnNeedInputBuffer(OH_AVCodec *codec, uint32_t index,
                                OH_AVBuffer *buffer, void *user_data) {
    CodecData *codec_data = static_cast<CodecData *>(user_data);
    if (codec_data == nullptr) {
      return;
    }
    std::unique_lock<std::mutex> lock(codec_data->in_mutex);
    codec_data->in_queue.emplace(index, buffer);
    codec_data->in_cond.notify_all();
  }

  static void OnNeedOutputBuffer(OH_AVCodec *codec, uint32_t index,
                                 OH_AVBuffer *buffer, void *user_data) {
    CodecData *codec_data = static_cast<CodecData *>(user_data);
    if (codec_data == nullptr) {
      return;
    }
    std::unique_lock<std::mutex> lock(codec_data->out_mutex);
    codec_data->out_queue.emplace(index, buffer);
    codec_data->out_cond.notify_all();
  }

  CodecData codec_data_;

  int32_t next_output_frame_ = 0;
  int32_t next_input_frame_ = 0;
};

}  // namespace

std::shared_ptr<VideoCodecManagerHarmony> MakeVideoCodecManagerAsyncHarmony(
    VideoData *data, OHNativeWindow *native_window) {
  return std::make_shared<VideoCodecManagerAsyncHarmony>(data, native_window);
}

}  // namespace animax
}  // namespace lynx
