// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_VIDEO_WEB_VIDEO_DECODER_WEB_H_
#define ANIMAX_SRC_VIDEO_WEB_VIDEO_DECODER_WEB_H_

#include <emscripten/val.h>

#include <memory>

#include "src/video/custom/video_decoder_custom.h"
#include "src/video/web/video_api_bridge.h"

namespace lynx {
namespace animax {

class VideoAssetWeb;

class VideoDecoderWeb final : public VideoDecoderCustom {
 public:
  VideoDecoderWeb();
  ~VideoDecoderWeb() override;

  bool IsValid() override;

  bool CreateDecoder(std::shared_ptr<VideoAsset> asset) override;

  bool DestroyDecoder() override;

  std::shared_ptr<YUVFrameInfo> DecodeFrameData(
      std::vector<uint8_t>& data,
      const std::shared_ptr<YUVFrameInfo>& reusable_frame) override;

 private:
  VideoDecoderPtr decoder_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_WEB_VIDEO_DECODER_WEB_H_
