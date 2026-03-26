// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/web/video_asset_web.h"

#include "src/base/log/log.h"
#include "src/base/monitor/trace_event.h"
#include "src/resource/asset/asset.h"
#include "src/resource/asset/video_asset_model.h"
#include "src/video/video_player.h"

namespace lynx {
namespace animax {

std::shared_ptr<VideoAsset> VideoAsset::Make(VideoAssetModel model) {
  return std::make_shared<VideoAssetWeb>(std::move(model));
}

// TODO(aiyongbiao.rick): Implement VideoAssetWeb methods.

VideoAssetWeb::VideoAssetWeb(VideoAssetModel model)
    : VideoAsset(std::move(model)) {}

std::vector<uint8_t> VideoAssetWeb::GetFrameRawData(int32_t frame) const {
  return {};
}

std::vector<uint8_t> VideoAssetWeb::GetVideoParameterSets() const { return {}; }

int VideoAssetWeb::GetPrevKeyFrame(const int32_t frame) const { return 0; }

bool VideoAssetWeb::PrepareFrameData(const std::string& video_path) {
  ANIMAX_TRACE_EVENT_ASSET_ID(kPrepareAlphaVideoFrameData, Model().id);
  is_valid_ = false;
  frame_count_ = 0;
  video_width_ = 1;
  video_height_ = 1;
  frame_rate_ = 0;
  ANIMAX_LOGI("PrepareFrameData finish, is_valid: "
              << std::to_string(IsValid()) << ", frame_count: " << frame_count_
              << ", video_width: " << video_width_ << ", video_height: "
              << video_height_ << ", frame_rate: " << frame_rate_);
  return IsValid();
}

}  // namespace animax
}  // namespace lynx
