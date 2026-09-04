// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <emscripten.h>

#include "ffmpeg_decoder_web.h"

extern "C" {

EMSCRIPTEN_KEEPALIVE
void* animax_video_decoder_alloc(const uint8_t* data, int32_t size) {
  if (!data || size <= 0) {
    return nullptr;
  }
  return lynx::animax::video_wasm::CreateVideoDecoder(data, size);
}

EMSCRIPTEN_KEEPALIVE
void animax_video_decoder_free(void* decoder) {
  if (!decoder) {
    return;
  }
  auto* decoder_ptr =
      static_cast<lynx::animax::video_wasm::FFmpegDecoderWeb*>(decoder);
  lynx::animax::video_wasm::DestroyVideoDecoder(decoder_ptr);
}

EMSCRIPTEN_KEEPALIVE
bool animax_video_prepare_frame_data(void* decoder, int32_t* width,
                                     int32_t* height, int32_t* frame_count,
                                     double* frame_rate, void* key_frames,
                                     void* frame_data) {
  if (!decoder) {
    return false;
  }

  auto* decoder_ptr =
      static_cast<lynx::animax::video_wasm::FFmpegDecoderWeb*>(decoder);
  auto* key_frames_ptr = static_cast<std::vector<int32_t>*>(key_frames);
  auto* frames_data_ptr =
      static_cast<std::vector<std::vector<uint8_t>>*>(frame_data);

  bool ret = lynx::animax::video_wasm::PrepareFrameData(
      decoder_ptr, key_frames_ptr, frames_data_ptr);
  if (ret) {
    *width = decoder_ptr->width;
    *height = decoder_ptr->height;
    *frame_rate = decoder_ptr->frame_rate;
    *frame_count = static_cast<int32_t>(frames_data_ptr->size());
  }

  return ret;
}

EMSCRIPTEN_KEEPALIVE
void* animax_video_frame_alloc(void* decoder, const uint8_t* data,
                               int32_t size) {
  if (!decoder || !data || size <= 0) {
    return nullptr;
  }
  auto* decoder_ptr =
      static_cast<lynx::animax::video_wasm::FFmpegDecoderWeb*>(decoder);
  return lynx::animax::video_wasm::DecodeFrame(decoder_ptr, data, size);
}

EMSCRIPTEN_KEEPALIVE
void animax_video_frame_free(void* frame) {
  if (!frame) {
    return;
  }
  auto* frame_ptr = static_cast<AVFrame*>(frame);
  lynx::animax::video_wasm::ReleaseFrame(frame_ptr);
}

EMSCRIPTEN_KEEPALIVE
bool animax_video_get_frame_data(void* frame, int32_t* width, int32_t* height,
                                 int32_t* format, int32_t (*yuv_linesize)[3],
                                 uint8_t* (*yuv_data)[3]) {
  if (!frame) {
    return false;
  }

  auto* frame_ptr = static_cast<AVFrame*>(frame);
  return lynx::animax::video_wasm::GetFrameData(frame_ptr, width, height,
                                                format, yuv_linesize, yuv_data);
}

}  // extern "C"
