// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/asset/video_asset.h"

#include <algorithm>

#include "src/base/log/log.h"
#include "src/base/util/misc_util.h"

namespace lynx {
namespace animax {

VideoAsset::VideoAsset(VideoAssetModel info) : model_{std::move(info)} {}

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

}  // namespace animax
}  // namespace lynx
