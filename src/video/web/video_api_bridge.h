// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_WEB_VIDEO_API_BRIDGE_H_
#define ANIMAX_SRC_VIDEO_WEB_VIDEO_API_BRIDGE_H_

#include <cstdint>
#include <memory>

// These functions are only declared in the main module. The actual
// implementations are provided by the side module (animax_video.wasm) and
// injected at runtime.
extern "C" {

/**
 * Allocates a video decoder.
 *
 * @param data Video data pointer
 * @param size Video data size
 * @return Decoder pointer, returns nullptr on failure
 */
void* animax_video_decoder_alloc(const uint8_t* data, int32_t size);

/**
 * Frees a video decoder.
 *
 * @param decoder Decoder pointer
 */
void animax_video_decoder_free(void* decoder);

/**
 * Prepares video frame data.
 *
 * @param decoder Decoder pointer
 * @param width Output video width
 * @param height Output video height
 * @param frame_count Output total frame count
 * @param frame_rate Output frame rate
 * @param key_frames Key frame list pointer. Although this is declared as void*,
 *                   it actually points to a std::vector<int32_t> object and
 * will be cast back to vector type in animax_video.wasm (side module).
 * @param frame_data Frame data list pointer. Although this is declared as
 * void*, it actually points to a std::vector<std::vector<uint8_t>> object and
 * will be cast back to vector type in animax_video.wasm (side module).
 * @return Returns true on success, false on failure
 *
 * @note Both the main module (animax_wasm.wasm) and the side module
 * (animax_video.wasm) are compiled with the same toolchain, which enables safe
 * casting of vector objects between them via void* pointers.
 */
bool animax_video_prepare_frame_data(void* decoder, int32_t* width,
                                     int32_t* height, int32_t* frame_count,
                                     double* frame_rate, void* key_frames,
                                     void* frame_data);

/**
 * Allocates a decoded frame.
 *
 * @param decoder Decoder pointer
 * @param data Frame data pointer
 * @param size Frame data size
 * @return Frame pointer, returns nullptr on failure
 */
void* animax_video_frame_alloc(void* decoder, const uint8_t* data,
                               int32_t size);

/**
 * Frees a frame.
 *
 * @param frame Frame pointer
 */
void animax_video_frame_free(void* frame);

/**
 * Gets frame data.
 *
 * @param frame Frame pointer
 * @param width Output frame width
 * @param height Output frame height
 * @param format Output format
 * @param yuv_linesize Output line size array
 * @param yuv_data Output data array
 * @return Returns true on success, false on failure
 */
bool animax_video_get_frame_data(void* frame, int32_t* width, int32_t* height,
                                 int32_t* format, int32_t (*yuv_linesize)[3],
                                 uint8_t* (*yuv_data)[3]);
}

namespace lynx {
namespace animax {

struct VideoDecoderDeleter {
  void operator()(void* decoder) const { animax_video_decoder_free(decoder); }
};

struct VideoFrameDeleter {
  void operator()(void* frame) const { animax_video_frame_free(frame); }
};

using VideoDecoderPtr = std::unique_ptr<void, VideoDecoderDeleter>;
using VideoFramePtr = std::unique_ptr<void, VideoFrameDeleter>;

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_WEB_VIDEO_API_BRIDGE_H_
