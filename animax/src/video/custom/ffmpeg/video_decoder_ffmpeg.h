// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_VIDEO_DECODER_FFMPEG_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_VIDEO_DECODER_FFMPEG_H_

#include <memory>

#include "include/player/animax_player.h"
#include "src/video/custom/video_decoder_custom.h"

namespace lynx {
namespace animax {

class VideoAssetFFmpeg;
class FFmpegVideoContext;

class VideoDecoderFFmpeg : public VideoDecoderCustom {
 public:
  VideoDecoderFFmpeg();
  ~VideoDecoderFFmpeg() override;

  bool IsValid() override;

  bool CreateDecoder(std::shared_ptr<VideoAsset> asset) override;

  bool DestroyDecoder() override;

  std::shared_ptr<YUVFrameInfo> DecodeFrameData(
      std::vector<uint8_t>& data,
      const std::shared_ptr<YUVFrameInfo>& reusable_frame) override;

 private:
  std::unique_ptr<FFmpegVideoContext> video_context_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_VIDEO_DECODER_FFMPEG_H_
