// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_CUSTOM_BYTEVC1_ANDROID_VIDEO_DECODER_BYTEVC1_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_BYTEVC1_ANDROID_VIDEO_DECODER_BYTEVC1_H_

#include "include/player/animax_player.h"
#include "src/base/gl/gl_include.h"
#include "src/video/custom/bytevc1/android/bytevc1_lib_loader.h"
#include "src/video/custom/video_decoder_custom.h"

namespace lynx {
namespace animax {

class VideoDecoderByteVc1 : public VideoDecoderCustom {
 public:
  VideoDecoderByteVc1();
  ~VideoDecoderByteVc1();

  std::shared_ptr<YUVFrameInfo> DecodeFrameData(
      std::vector<uint8_t>& data,
      const std::shared_ptr<YUVFrameInfo>& reusable_frame) override;
  bool CreateDecoder(std::shared_ptr<VideoAsset> asset) override;
  bool DestroyDecoder() override;
  bool IsValid() override;

 private:
  ByteVC1LibLoader::DecoderPtr decoder_ptr_ = nullptr;
  mutable std::shared_mutex mutex_;
  bool is_asset_valid_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_BYTEVC1_ANDROID_VIDEO_DECODER_BYTEVC1_H_
