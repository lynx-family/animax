// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/video/custom/ffmpeg/video_asset_ffmpeg.h"

#include <algorithm>

#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/video/custom/ffmpeg/ffmpeg_lib_loader.h"
#include "src/video/custom/ffmpeg/ffmpeg_video_context.h"

namespace lynx {
namespace animax {

std::shared_ptr<VideoAsset> VideoAsset::Make(VideoAssetModel model) {
  return std::make_shared<VideoAssetFFmpeg>(model);
}

VideoAssetFFmpeg::VideoAssetFFmpeg(VideoAssetModel model) : VideoAsset(model) {}

bool VideoAssetFFmpeg::PrepareFrameData(const std::string& video_path) {
  ANIMAX_LOGI("Preparing frame data for video: " << video_path);

  FFmpegVideoContext context(video_path);
  if (!context.IsValid()) {
    ANIMAX_LOGE("Failed to create video context");
    return false;
  }

  video_path_ = video_path;
  frames_data_.clear();
  key_frames_.clear();

  if (!FFmpegLibLoader::Instance().ReadVideoFramesData(&context, frames_data_,
                                                       key_frames_)) {
    ANIMAX_LOGE("Failed to read video frames data");
    return false;
  }

  video_width_ = context.GetWidth();
  video_height_ = context.GetHeight();
  frame_rate_ = context.GetFrameRate();
  frame_count_ = static_cast<int32_t>(frames_data_.size());

  ANIMAX_LOGI("Frame data prepared - " << frame_count_ << " frames, "
                                       << video_width_ << "x" << video_height_
                                       << ", " << frame_rate_ << " frame_rate");

  bool is_valid = video_width_ > 0 && video_height_ > 0 && frame_count_ > 0;
  is_valid_.store(is_valid);
  return is_valid;
}

int VideoAssetFFmpeg::GetPrevKeyFrame(const int32_t frame) const {
  if (key_frames_.empty()) {
    return 0;
  }
  auto it = std::upper_bound(key_frames_.begin(), key_frames_.end(), frame);
  if (it == key_frames_.begin()) {
    return *it;
  }
  return *--it;
}

std::vector<uint8_t> VideoAssetFFmpeg::GetVideoParameterSets() const {
  // FFmpeg does not support parameter sets.
  // Return an empty vector.
  return {};
}

std::vector<uint8_t> VideoAssetFFmpeg::GetFrameRawData(int32_t frame) const {
  if (frame < 0 || frame >= frame_count_) {
    return {};
  }

  return frames_data_[frame];
}

}  // namespace animax
}  // namespace lynx
