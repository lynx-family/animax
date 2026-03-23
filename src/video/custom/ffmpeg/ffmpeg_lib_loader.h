// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_FFMPEG_LIB_LOADER_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_FFMPEG_LIB_LOADER_H_

#include <Windows.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "base/include/no_destructor.h"
#include "src/video/custom/ffmpeg/ffmpeg_video_context.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

namespace lynx {
namespace animax {

class YUVFrameInfo;

class FFmpegLibLoader {
 public:
  /**
   * Gets the singleton instance of FFmpegLibLoader.
   * @return Reference to the singleton instance.
   */
  static FFmpegLibLoader& Instance();
  /**
   * Checks if the FFmpeg library is successfully loaded and all symbols are
   * valid.
   * @return True if the library is valid, false otherwise.
   */
  bool IsValid() const;

  /**
   * Decodes video frame data using the provided video context.
   * @param video_context Pointer to the FFmpegVideoContext containing codec
   * information.
   * @param data Pointer to the encoded frame data.
   * @param data_size Size of the encoded frame data.
   * @return A FramePtr containing the decoded AVFrame, or nullptr if decoding
   * fails.
   */
  FFmpegVideoContext::FramePtr DecodeFrameData(
      const FFmpegVideoContext* video_context, const uint8_t* data,
      size_t data_size);

  /**
   * Reads all video frames and keyframe indices from the provided video
   * context.
   * @param video_context Pointer to the FFmpegVideoContext.
   * @param frames_data Reference to a vector where the raw frame data will be
   * stored.
   * @param key_frames Reference to a vector where the keyframe indices will be
   * stored.
   * @return True if frames were successfully read, false otherwise.
   */
  bool ReadVideoFramesData(const FFmpegVideoContext* video_context,
                           std::vector<std::vector<uint8_t>>& frames_data,
                           std::vector<int32_t>& key_frames);

  /**
   * Frees an AVFormatContext and its associated resources.
   * @param ctx Pointer to a pointer of the AVFormatContext to be freed.
   */
  void FreeFormatContext(AVFormatContext** ctx);

  /**
   * Frees an AVCodecContext and its associated resources.
   * @param ctx Pointer to a pointer of the AVCodecContext to be freed.
   */
  void FreeCodecContext(AVCodecContext** ctx);

  /**
   * Frees an AVPacket and its associated resources.
   * @param packet Pointer to a pointer of the AVPacket to be freed.
   */
  void FreePacket(AVPacket** packet);

  /**
   * Frees an AVFrame and its associated resources.
   * @param frame Pointer to a pointer of the AVFrame to be freed.
   */
  void FreeFrame(AVFrame** frame);

 private:
  friend class base::NoDestructor<FFmpegLibLoader>;
  friend class FFmpegVideoContext;

  FFmpegLibLoader();
  ~FFmpegLibLoader() = default;

  bool LoadLibrary();
  bool LoadAllSymbols();

  HMODULE dll_ = nullptr;
  bool is_symbols_valid_ = false;

  // Function pointers
  decltype(&avformat_open_input) avformat_open_input_func_ = nullptr;
  decltype(&avformat_close_input) avformat_close_input_func_ = nullptr;
  decltype(&avformat_find_stream_info) avformat_find_stream_info_func_ =
      nullptr;
  decltype(&avcodec_find_decoder) avcodec_find_decoder_func_ = nullptr;
  decltype(&avcodec_alloc_context3) avcodec_alloc_context3_func_ = nullptr;
  decltype(&avcodec_parameters_to_context) avcodec_parameters_to_context_func_ =
      nullptr;
  decltype(&avcodec_open2) avcodec_open2_func_ = nullptr;
  decltype(&avcodec_free_context) avcodec_free_context_func_ = nullptr;
  decltype(&av_packet_alloc) av_packet_alloc_func_ = nullptr;
  decltype(&av_new_packet) av_new_packet_func_ = nullptr;
  decltype(&av_packet_free) av_packet_free_func_ = nullptr;
  decltype(&av_frame_alloc) av_frame_alloc_func_ = nullptr;
  decltype(&av_frame_free) av_frame_free_func_ = nullptr;
  decltype(&av_read_frame) av_read_frame_func_ = nullptr;
  decltype(&avcodec_send_packet) avcodec_send_packet_func_ = nullptr;
  decltype(&avcodec_receive_frame) avcodec_receive_frame_func_ = nullptr;
  decltype(&av_packet_unref) av_packet_unref_func_ = nullptr;
  decltype(&av_seek_frame) av_seek_frame_func_ = nullptr;
  decltype(&avformat_network_init) avformat_network_init_func_ = nullptr;
  decltype(&av_log_set_callback) av_log_set_callback_func_ = nullptr;
  decltype(&av_log_set_level) av_log_set_level_func_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_FFMPEG_LIB_LOADER_H_
