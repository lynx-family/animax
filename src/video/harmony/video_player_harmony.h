// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_HARMONY_VIDEO_PLAYER_HARMONY_H_
#define ANIMAX_SRC_VIDEO_HARMONY_VIDEO_PLAYER_HARMONY_H_

#include <multimedia/player_framework/native_avcodec_videodecoder.h>
#include <native_image/native_image.h>
#include <native_window/external_window.h>

#include <condition_variable>
#include <mutex>
#include <queue>

#include "src/render/texture_info_gl.h"
#include "src/video/harmony/video_asset_harmony.h"
#include "src/video/video_player.h"

namespace lynx {
namespace animax {

/**
 * Codec buffer info callback from OnNeedInputBuffer or OnNeedOutputBuffer
 * The OH_AVCodecBufferAttr will be initialized during creation.
 */
struct CodecBufferInfo {
  uint32_t index = 0;
  OH_AVBuffer *buffer = nullptr;
  OH_AVCodecBufferAttr attr = {0, 0, 0, AVCODEC_BUFFER_FLAGS_NONE};

  CodecBufferInfo(uint32_t arg_index, OH_AVBuffer *arg_buffer)
      : index(arg_index), buffer(arg_buffer) {
    OH_AVBuffer_GetBufferAttr(buffer, &attr);
  };

  uint8_t *GetAddr() { return OH_AVBuffer_GetAddr(buffer); }
};

/**
 * The data struct used by Codec, will be registered on
 * OH_VideoDecoder_RegisterCallback In each input and ouput buffer callback, the
 * index must put into queue and wait for later process.
 */
struct CodecData {
  std::mutex in_mutex;
  std::condition_variable in_cond;
  std::queue<CodecBufferInfo> in_queue;

  std::mutex out_mutex;
  std::condition_variable out_cond;
  std::queue<CodecBufferInfo> out_queue;
};

class VideoPlayerHarmony : public VideoPlayer {
 public:
  VideoPlayerHarmony();
  ~VideoPlayerHarmony() override;

  static constexpr const std::chrono::milliseconds kBaseTimeout{16};
  static constexpr const int32_t kMaxTimeoutCount = 5;
  static constexpr const int32_t kSurfaceUpdateRetryCount = 2;

  static void OnError(OH_AVCodec *codec, int32_t errorCode, void *userData);

  static void OnStreamChanged(OH_AVCodec *codec, OH_AVFormat *format,
                              void *userData);

  static void OnNeedInputBuffer(OH_AVCodec *codec, uint32_t index,
                                OH_AVBuffer *buffer, void *userData);

  static void OnNeedOutputBuffer(OH_AVCodec *codec, uint32_t index,
                                 OH_AVBuffer *buffer, void *userData);

  std::unique_ptr<TextureInfo> UpdateTexture(const int32_t frame) override;

  void UpdateSurfaceImage();

  const std::array<float, 16> &GetTransform() override;

  void AttachAsset(VideoAsset *asset) override;

  void NotifyErrorEvent(const std::string &err_msg);

  // Render target frame, will push input buffer into codec and render the
  // output buffer.
  bool RenderFrame(int32_t frame);

  // Wait the input queue and copy video data into buffer and send to codec to
  // decode.
  bool DecodeFrame(int32_t frame);

  // Wait the output queue and render the texture to surface.
  bool ProcessOutputFrame(bool render = true);

 private:
  // Create the native window by creating texture id and native image.
  void InitNativeWindow();

  // Create and start the codec, the native window will attach to the codec.
  void InitCodec();

  // Get the starting keyframe index for the given frame.
  int32_t GetPrevKeyframe(int32_t frame) const;

  // Update next input and output frame counters based on the starting keyframe
  // index.
  void UpdateFrameCounters(int32_t frame);

  // Increase the frame counter. If it reaches the end frame, it will start from
  // 0.
  int32_t AdvanceFrameCounter(int32_t frame);

  // Get the timeout threshold for waiting for input and output buffers.
  std::chrono::milliseconds GetTimeout(int32_t frame) const;

  // Increase the timeout occurrence count.
  void IncreaseTimeoutCount();

  uint32_t video_texture_ = 0;
  std::array<float, 16> transform_{};

  VideoAssetHarmony *asset_ = nullptr;
  VideoData *data_ = nullptr;

  OH_AVCodec *av_codec_ = nullptr;
  OH_NativeImage *native_image_ = nullptr;
  OHNativeWindow *native_window_ = nullptr;

  CodecData codec_data_;

  int32_t next_output_frame_ = 0;
  int32_t next_input_frame_ = 0;
  int32_t timeout_count_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_HARMONY_VIDEO_PLAYER_HARMONY_H_
