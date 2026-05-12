// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_HARMONY_VIDEO_DECODER_SYNC_API_HARMONY_H_
#define ANIMAX_SRC_BASE_UTIL_HARMONY_VIDEO_DECODER_SYNC_API_HARMONY_H_

#include <multimedia/player_framework/native_avcodec_videodecoder.h>
#include <multimedia/player_framework/native_avformat.h>

#include <cstdint>

#include "base/include/no_destructor.h"

namespace lynx {
namespace animax {

using QueryVideoDecoderBufferFunc = OH_AVErrCode(OH_AVCodec *codec,
                                                 uint32_t *index,
                                                 int64_t timeout_us);
using GetVideoDecoderBufferFunc = OH_AVBuffer *(OH_AVCodec *codec,
                                                uint32_t index);

class HarmonyVideoDecoderSyncApi {
 public:
  static HarmonyVideoDecoderSyncApi &GetInstance();

  HarmonyVideoDecoderSyncApi(const HarmonyVideoDecoderSyncApi &) = delete;
  HarmonyVideoDecoderSyncApi &operator=(const HarmonyVideoDecoderSyncApi &) =
      delete;

  OH_AVErrCode QueryInputBuffer(OH_AVCodec *codec, uint32_t *index,
                                int64_t timeout_us);
  OH_AVBuffer *GetInputBuffer(OH_AVCodec *codec, uint32_t index);
  OH_AVErrCode QueryOutputBuffer(OH_AVCodec *codec, uint32_t *index,
                                 int64_t timeout_us);
  OH_AVBuffer *GetOutputBuffer(OH_AVCodec *codec, uint32_t index);
  bool ConfigureSyncMode(OH_AVFormat *format, bool enable);
  bool IsSupported();

 private:
  friend class lynx::base::NoDestructor<HarmonyVideoDecoderSyncApi>;

  HarmonyVideoDecoderSyncApi();
  bool LoadSyncApi();

  bool is_supported_ = false;
  QueryVideoDecoderBufferFunc *query_input_buffer_ = nullptr;
  GetVideoDecoderBufferFunc *get_input_buffer_ = nullptr;
  QueryVideoDecoderBufferFunc *query_output_buffer_ = nullptr;
  GetVideoDecoderBufferFunc *get_output_buffer_ = nullptr;
  const char **enable_sync_mode_key_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_HARMONY_VIDEO_DECODER_SYNC_API_HARMONY_H_
