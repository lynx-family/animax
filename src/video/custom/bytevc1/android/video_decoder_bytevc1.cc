// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/bytevc1/android/video_decoder_bytevc1.h"

#include "animax/animax/src/resource/asset/asset.h"
#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/base/log/log.h"
#include "src/resource/asset/video_asset.h"

namespace lynx {
namespace animax {

static int ByteVC1LogCallback(void *avl, int level, const char *fmt, ...) {
  char buffer[1024];
  va_list arg;
  va_start(arg, fmt);
  vsnprintf(buffer, 1024, fmt, arg);
  va_end(arg);
  ANIMAX_LOGE(buffer);
  return 0;
}

VideoDecoderByteVc1::VideoDecoderByteVc1() {}

VideoDecoderByteVc1::~VideoDecoderByteVc1() { DestroyDecoder(); }

bool VideoDecoderByteVc1::IsValid() {
  return ByteVC1LibLoader::Instance().IsValid() && is_asset_valid_;
}

bool VideoDecoderByteVc1::CreateDecoder(std::shared_ptr<VideoAsset> asset) {
  if (decoder_ptr_) {
    return false;
  }

  std::unique_lock lock(mutex_);
  decoder_ptr_ = ByteVC1LibLoader::Instance().CreateDecoder();
  is_asset_valid_ = !asset->GetVideoParameterSets().empty();
  if (!decoder_ptr_ || !is_asset_valid_) {
    return false;
  }

  ByteVC1LibLoader::Instance().SetLogCallback(decoder_ptr_, ByteVC1LogCallback);
  return true;
}

bool VideoDecoderByteVc1::DestroyDecoder() {
  if (!IsValid()) {
    return false;
  }

  std::unique_lock lock(mutex_);
  decoder_ptr_ = nullptr;
  return true;
}

std::shared_ptr<YUVFrameInfo> VideoDecoderByteVc1::DecodeFrameData(
    std::vector<uint8_t> &buffer,
    const std::shared_ptr<YUVFrameInfo> &reusable_frame) {
  if (!IsValid() || buffer.empty()) {
    return nullptr;
  }

  auto packet_ptr = ByteVC1LibLoader::Instance().CreatePacket(buffer);
  if (!packet_ptr) {
    return nullptr;
  }

  std::shared_lock lock(mutex_);
  auto frame_ptr =
      ByteVC1LibLoader::Instance().DecodePacket(decoder_ptr_, packet_ptr);
  lock.unlock();

  if (!frame_ptr) {
    return nullptr;
  }

  auto frame_info =
      reusable_frame ? reusable_frame : std::make_shared<YUVFrameInfo>();
  ByteVC1LibLoader::Instance().ExtractFrameData(frame_ptr, frame_info);
  return frame_info;
}

}  // namespace animax
}  // namespace lynx
