// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/web/video_asset_web.h"

#include <algorithm>

#include "src/base/log/log.h"
#include "src/resource/asset/asset.h"
#include "src/resource/asset/video_asset_model.h"
#include "src/resource/resource_loader/web/resource_loader_web.h"
#include "src/video/video_player.h"
#include "src/video/web/video_api_bridge.h"

namespace lynx {
namespace animax {

std::shared_ptr<VideoAsset> VideoAsset::Make(VideoAssetModel model) {
  return std::make_shared<VideoAssetWeb>(std::move(model));
}

VideoAssetWeb::VideoAssetWeb(VideoAssetModel model)
    : VideoAsset(std::move(model)) {}

std::vector<uint8_t> VideoAssetWeb::GetFrameRawData(int32_t frame) const {
  if (frame < 0 || frame >= frame_count_) {
    return {};
  }
  return frames_data_[frame];
}

bool VideoAssetWeb::PrepareFrameData(const std::string& video_path) {
  // On web platform, video asset cannot be loaded using local path
  ANIMAX_LOGE("Video asset cannot be loaded using local path: " << video_path);
  DCHECK(false);
  return false;
}

bool VideoAssetWeb::PrepareFrameData(std::unique_ptr<RawData> raw_data) {
  if (!raw_data) {
    ANIMAX_LOGE("Raw data is null");
    return false;
  }

  frames_data_.clear();
  key_frames_.clear();
  raw_data_ = std::move(raw_data);

  VideoDecoderPtr decoder(
      animax_video_decoder_alloc(static_cast<const uint8_t*>(raw_data_->Data()),
                                 static_cast<int32_t>(raw_data_->Length())));
  if (!decoder) {
    ANIMAX_LOGE("Failed to create video decoder");
    return false;
  }

  if (!animax_video_prepare_frame_data(
          decoder.get(), &video_width_, &video_height_, &frame_count_,
          &frame_rate_, &key_frames_, &frames_data_)) {
    ANIMAX_LOGE("Failed to prepare video frame data");
    return false;
  }

  is_valid_.store(true);
  ANIMAX_LOGI("PrepareFrameData finish, is_valid: "
              << std::to_string(IsValid()) << ", frame_count: " << frame_count_
              << ", video_width: " << video_width_ << ", video_height: "
              << video_height_ << ", frame_rate: " << frame_rate_);
  return IsValid();
}

std::vector<uint8_t> VideoAssetWeb::GetVideoParameterSets() const {
  // FFmpeg does not support parameter sets.
  // Return an empty vector.
  return {};
}

int VideoAssetWeb::GetPrevKeyFrame(const int32_t frame) const {
  if (key_frames_.empty()) {
    return 0;
  }
  auto it = std::upper_bound(key_frames_.begin(), key_frames_.end(), frame);
  if (it == key_frames_.begin()) {
    return 0;
  }
  return *--it;
}

}  // namespace animax
}  // namespace lynx
