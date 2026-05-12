// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/harmony/video_decoder_sync_api_harmony.h"

#include <deviceinfo.h>
#include <dlfcn.h>

#include "src/base/log/log.h"
#include "src/base/util/symbol_util.h"

namespace lynx {
namespace animax {
namespace {

constexpr int32_t kHarmonySyncDecoderApiLevel = 20;

void *GetNativeMediaVdecHandle() {
  static void *handle =
      dlopen("libnative_media_vdec.so", RTLD_NOW | RTLD_LOCAL);
  if (!handle) {
    ANIMAX_LOGE("Failed to open libnative_media_vdec.so")
  }
  return handle ? handle : RTLD_DEFAULT;
}

void *GetNativeMediaCodecBaseHandle() {
  static void *handle =
      dlopen("libnative_media_codecbase.so", RTLD_NOW | RTLD_LOCAL);
  if (!handle) {
    ANIMAX_LOGE("Failed to open libnative_media_codecbase.so")
  }
  return handle ? handle : RTLD_DEFAULT;
}

}  // namespace

HarmonyVideoDecoderSyncApi::HarmonyVideoDecoderSyncApi() {
  int api_version = OH_GetSdkApiVersion();
  if (api_version < kHarmonySyncDecoderApiLevel) {
    ANIMAX_LOGI(
        "Harmony sync video decoder is disabled, api version: " << api_version);
    return;
  }

  is_supported_ = LoadSyncApi();
  if (!is_supported_) {
    ANIMAX_LOGW("Harmony sync video decoder symbols are incomplete");
    return;
  }

  ANIMAX_LOGI(
      "Harmony sync video decoder is enabled, api version: " << api_version);
}

bool HarmonyVideoDecoderSyncApi::IsSupported() { return is_supported_; }

HarmonyVideoDecoderSyncApi &HarmonyVideoDecoderSyncApi::GetInstance() {
  static lynx::base::NoDestructor<HarmonyVideoDecoderSyncApi> api;
  return *api;
}

OH_AVErrCode HarmonyVideoDecoderSyncApi::QueryInputBuffer(OH_AVCodec *codec,
                                                          uint32_t *index,
                                                          int64_t timeout_us) {
  if (!query_input_buffer_) {
    return OH_AVErrCode::AV_ERR_UNSUPPORT;
  }
  return query_input_buffer_(codec, index, timeout_us);
}

OH_AVBuffer *HarmonyVideoDecoderSyncApi::GetInputBuffer(OH_AVCodec *codec,
                                                        uint32_t index) {
  if (!get_input_buffer_) {
    return nullptr;
  }
  return get_input_buffer_(codec, index);
}

OH_AVErrCode HarmonyVideoDecoderSyncApi::QueryOutputBuffer(OH_AVCodec *codec,
                                                           uint32_t *index,
                                                           int64_t timeout_us) {
  if (!query_output_buffer_) {
    return OH_AVErrCode::AV_ERR_UNSUPPORT;
  }
  return query_output_buffer_(codec, index, timeout_us);
}

OH_AVBuffer *HarmonyVideoDecoderSyncApi::GetOutputBuffer(OH_AVCodec *codec,
                                                         uint32_t index) {
  if (!get_output_buffer_) {
    return nullptr;
  }
  return get_output_buffer_(codec, index);
}

bool HarmonyVideoDecoderSyncApi::ConfigureSyncMode(OH_AVFormat *format,
                                                   bool enable) {
  if (format == nullptr || enable_sync_mode_key_ == nullptr ||
      *enable_sync_mode_key_ == nullptr) {
    return false;
  }

  return OH_AVFormat_SetIntValue(format, *enable_sync_mode_key_,
                                 enable ? 1 : 0);
}

bool HarmonyVideoDecoderSyncApi::LoadSyncApi() {
  void *vdec_handle = GetNativeMediaVdecHandle();
  void *codecbase_handle = GetNativeMediaCodecBaseHandle();
  return SymbolUtil::LoadSymbol(vdec_handle, query_input_buffer_,
                                "OH_VideoDecoder_QueryInputBuffer") &&
         SymbolUtil::LoadSymbol(vdec_handle, get_input_buffer_,
                                "OH_VideoDecoder_GetInputBuffer") &&
         SymbolUtil::LoadSymbol(vdec_handle, query_output_buffer_,
                                "OH_VideoDecoder_QueryOutputBuffer") &&
         SymbolUtil::LoadSymbol(vdec_handle, get_output_buffer_,
                                "OH_VideoDecoder_GetOutputBuffer") &&
         SymbolUtil::LoadSymbol(codecbase_handle, enable_sync_mode_key_,
                                "OH_MD_KEY_ENABLE_SYNC_MODE");
}

}  // namespace animax
}  // namespace lynx
