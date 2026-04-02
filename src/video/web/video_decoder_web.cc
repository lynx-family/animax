// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/video/web/video_decoder_web.h"

#include "src/base/log/log.h"
#include "src/video/custom/yuv_frame_info.h"
#include "src/video/web/video_api_bridge.h"
#include "src/video/web/video_asset_web.h"

namespace lynx {
namespace animax {

VideoDecoderWeb::VideoDecoderWeb() {}

VideoDecoderWeb::~VideoDecoderWeb() { DestroyDecoder(); }

bool VideoDecoderWeb::IsValid() { return decoder_ != nullptr; }

bool VideoDecoderWeb::CreateDecoder(std::shared_ptr<VideoAsset> asset) {
  auto asset_web = std::static_pointer_cast<VideoAssetWeb>(asset);
  if (!asset_web) {
    ANIMAX_LOGE("Invalid video asset");
    return false;
  }

  auto decoder = animax_video_decoder_alloc(
      static_cast<const uint8_t*>(asset_web->GetRawData()->Data()),
      static_cast<int32_t>(asset_web->GetRawData()->Length()));
  if (!decoder) {
    ANIMAX_LOGE("Failed to create decoder");
    return false;
  }
  decoder_.reset(decoder);
  ANIMAX_LOGI("Decoder created");
  return true;
}

bool VideoDecoderWeb::DestroyDecoder() {
  ANIMAX_LOGI("Decoder destroyed");
  decoder_.reset();
  return true;
}

std::shared_ptr<YUVFrameInfo> VideoDecoderWeb::DecodeFrameData(
    std::vector<uint8_t>& data,
    const std::shared_ptr<YUVFrameInfo>& reusable_frame) {
  if (!IsValid() || !decoder_) {
    return nullptr;
  }

  VideoFramePtr frame(animax_video_frame_alloc(
      decoder_.get(), data.data(), static_cast<int32_t>(data.size())));
  if (!frame) {
    return nullptr;
  }

  int32_t width, height, format;
  int32_t yuv_linesize[3];
  uint8_t* yuv_data[3];

  if (!animax_video_get_frame_data(frame.get(), &width, &height, &format,
                                   &yuv_linesize, &yuv_data)) {
    return nullptr;
  }

  auto frame_info = reusable_frame != nullptr
                        ? reusable_frame
                        : std::make_shared<YUVFrameInfo>();

  frame_info->SetWidth(width);
  frame_info->SetHeight(height);
  frame_info->SetYUVPixFmt(static_cast<YUVPixFmt>(format));
  for (int i = 0; i < YUVFrameInfo::kYUVChannels; i++) {
    frame_info->SetLineSize(i, yuv_linesize[i]);
    frame_info->UpdateChannelData(i, yuv_data[i]);
  }

  return frame_info;
}

}  // namespace animax
}  // namespace lynx
