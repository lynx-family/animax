// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/asset/video_asset.h"

#include <algorithm>
#include <cmath>

#include "src/base/log/log.h"
#include "src/base/util/misc_util.h"

namespace lynx {
namespace animax {

VideoAsset::VideoAsset(VideoAssetModel info) : model_{std::move(info)} {
  ANIMAX_LOGI("Video asset is created, this: " << this);
}

VideoAsset::~VideoAsset() {
  ANIMAX_LOGI("Video asset is destroyed, this: " << this);
}

int64_t VideoAsset::GetMemoryUsageBytes() const {
  return sizeof(VideoAsset) + model_.id.capacity() +
         model_.dir_name.capacity() + model_.file_name.capacity();
}

std::string VideoAsset::ComposeUrl(const std::string& prefix) const {
  std::string full_path;
  full_path.append(Model().dir_name).append(Model().file_name);
  std::string full_url;
  if (IsUrlValid(full_path)) {
    full_url = full_path;
  } else {
    full_url.append(prefix).append(full_path);
  }

  return full_url;
}

void VideoAsset::LoadLocal(const std::string& path) {
  if (path.empty() || IsValid()) {
    return;
  }
  PrepareFrameData(path);
}

void VideoAsset::RecoverFrameIfNecessary() {
  auto p = [](int32_t v) { return 0 == v; };
  if (!std::all_of(Model().rgb_frame.begin(), Model().rgb_frame.end(), p) ||
      !std::all_of(Model().a_frame.begin(), Model().a_frame.end(), p)) {
    return;
  }
  ANIMAX_LOGE("config.json is incomplete");
  if (0 == video_width_ || (video_width_ & 1)) {
    ANIMAX_LOGE("can't recover rgbFrame or aFrame");
    return;
  }
  model_.a_frame = {0, 0, video_width_ >> 1, video_height_};
  model_.rgb_frame = {video_width_ >> 1, 0, video_width_ >> 1, video_height_};
}

void VideoAsset::ResetModel(VideoAssetModel info) {
  model_ = std::move(info);
  video_width_ = 0;
  video_height_ = 0;
  frame_rate_ = 0.f;
  is_valid_ = false;
}

LoaderError VideoAsset::ResolvePreparedModel(VideoAssetModel* model) const {
  if (!model || !IsValid() || video_width_ <= 0 || video_height_ <= 0 ||
      !std::isfinite(GetFrameRate()) || GetFrameRate() <= 0.f ||
      GetFrameCount() <= 0) {
    return {kInvalidRawData, "Prepared video is invalid"};
  }

  auto resolved = *model;
  if (resolved.rgb_frame == std::array<int32_t, 4>{} &&
      resolved.a_frame == std::array<int32_t, 4>{}) {
    if (video_width_ & 1) {
      return {kInvalidRawData,
              "Cannot recover video regions from an odd width"};
    }
    resolved.a_frame = {0, 0, video_width_ / 2, video_height_};
    resolved.rgb_frame = {video_width_ / 2, 0, video_width_ / 2, video_height_};
  }

  const auto valid_region = [this](const std::array<int32_t, 4>& region) {
    return region[0] >= 0 && region[1] >= 0 && region[2] > 0 && region[3] > 0 &&
           static_cast<int64_t>(region[0]) + region[2] <= video_width_ &&
           static_cast<int64_t>(region[1]) + region[3] <= video_height_;
  };
  if (!valid_region(resolved.rgb_frame) || !valid_region(resolved.a_frame)) {
    return {kInvalidRawData,
            "Video regions exceed the prepared video dimensions"};
  }

  *model = std::move(resolved);
  return {};
}

LoaderError VideoAsset::SetPreparedModel(VideoAssetModel model) {
  auto error = ResolvePreparedModel(&model);
  if (error) {
    return error;
  }
  model_ = std::move(model);
  return {};
}

}  // namespace animax
}  // namespace lynx
