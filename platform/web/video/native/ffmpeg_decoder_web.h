// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_PLATFORM_WEB_VIDEO_NATIVE_FFMPEG_DECODER_WEB_H_
#define ANIMAX_PLATFORM_WEB_VIDEO_NATIVE_FFMPEG_DECODER_WEB_H_

#include <cstdint>
#include <vector>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/avutil.h"
}

#define ANIMAX_VIDEO_WASM_LOGE(fmt, ...) \
  fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)

namespace lynx {
namespace animax {
namespace video_wasm {

/**
 * FFmpeg video decoder for WebAssembly (WASM) platform.
 *
 * Holds all resources needed for video decoding from in-memory
 * data, including AVFormatContext (demuxer), AVCodecContext (decoder), and
 * custom AVIO callbacks for memory-based I/O.
 *
 * Destructor releases all owned resources (codec context, format context with
 * its internal AVIOContext and buffer). Safe to destroy a partially
 * initialized instance via delete.
 */
struct FFmpegDecoderWeb {
  FFmpegDecoderWeb() = default;
  ~FFmpegDecoderWeb();
  FFmpegDecoderWeb(const FFmpegDecoderWeb&) = delete;
  FFmpegDecoderWeb& operator=(const FFmpegDecoderWeb&) = delete;

  AVFormatContext* format_context = nullptr;
  AVCodecContext* codec_context = nullptr;
  int video_stream_index = -1;

  const uint8_t* data = nullptr;
  int64_t data_size = 0;
  int64_t data_position = 0;

  int32_t width = 0;
  int32_t height = 0;
  double frame_rate = 0.0;
};

/**
 * Create a video decoder for in-memory video data.
 *
 * Sets up AVFormatContext with custom AVIO memory callbacks, finds the video
 * stream, opens the appropriate decoder, and returns a
 * fully initialized decoder handle.
 *
 * @param data   Pointer to the raw video file data (MP4/MKV bytes).
 *               Must remain valid for the lifetime of the returned decoder.
 * @param size   Size of the data buffer in bytes.
 * @return       Initialized decoder on success, nullptr on failure.
 *               Caller must free with DestroyVideoDecoder().
 */
FFmpegDecoderWeb* CreateVideoDecoder(const uint8_t* data, int32_t size);

/**
 * Destroy a video decoder and release all associated resources.
 *
 * Calls the destructor which frees AVCodecContext and AVFormatContext
 * (including its internal AVIOContext and buffer), then deletes the struct.
 * Safe to call with nullptr.
 *
 * @param decoder  Decoder to destroy. May be nullptr.
 */
void DestroyVideoDecoder(FFmpegDecoderWeb* decoder);

/**
 * Extract all video frame packets from the decoder's input data.
 *
 * Iterates through all packets in the demuxed stream, collecting raw
 * packet data and recording key-frame indices. After calling this,
 * the decoder's read position is at end-of-stream; use DecodeFrame()
 * for individual frame decoding instead of re-reading from source.
 *
 * @param decoder      Valid decoder created by CreateVideoDecoder().
 * @param key_frames   [out] Indices of key frames within the frame list.
 * @param frames_data  [out] Raw packet data for each video frame.
 * @return             true on success, false on invalid arguments.
 */
bool PrepareFrameData(FFmpegDecoderWeb* decoder,
                      std::vector<int32_t>* key_frames,
                      std::vector<std::vector<uint8_t>>* frames_data);

/**
 * Decode a single video frame from raw packet data.
 *
 * Sends the packet to the decoder and receives one decoded frame.
 * For multi-layer codecs like H.265, multiple calls may be needed
 * per output frame (returns nullptr with EAGAIN internally).
 *
 * @param decoder  Valid decoder with an opened codec context.
 * @param data     Raw packet data (one encoded frame).
 * @param size     Size of the packet data in bytes.
 * @return         Decoded AVFrame on success, nullptr on failure or EOF.
 *                 Caller must free with ReleaseFrame().
 */
AVFrame* DecodeFrame(FFmpegDecoderWeb* decoder, const uint8_t* data,
                     int32_t size);

/**
 * Extract YUV pixel data from a decoded frame.
 *
 * Reads dimensions, pixel format, linesize pointers, and plane data
 * pointers from the given AVFrame into plain C-compatible output params.
 *
 * @param frame         Decoded frame from DecodeFrame(). May be nullptr.
 * @param width         [out] Frame width in pixels.
 * @param height        [out] Frame height in pixels.
 * @param format        [out] Pixel format: 1=YUV420P, 2=YUV422P, 3=YUV444P,
 * 0=other.
 * @param yuv_linesize  [out] Array of 3 linesize values (bytes per row per
 * plane).
 * @param yuv_data      [out] Array of 3 plane data pointers.
 * @return              true on success, false on null frame or null outputs.
 */
bool GetFrameData(AVFrame* frame, int32_t* width, int32_t* height,
                  int32_t* format, int32_t (*yuv_linesize)[3],
                  uint8_t* (*yuv_data)[3]);

/**
 * Release a decoded frame and free its memory.
 *
 * Safe to call with nullptr.
 *
 * @param frame  Frame to release. May be nullptr.
 */
void ReleaseFrame(AVFrame* frame);

}  // namespace video_wasm
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_PLATFORM_WEB_VIDEO_NATIVE_FFMPEG_DECODER_WEB_H_
