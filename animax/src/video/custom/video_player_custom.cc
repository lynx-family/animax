// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/video_player_custom.h"

#include "src/base/gl/gl_include.h"
#include "src/base/log/log.h"
#include "src/render/texture_info_frame_data.h"
#include "src/resource/asset/video_asset.h"
#include "src/video/custom/video_decoder_custom.h"

namespace lynx {
namespace animax {

VideoPlayerCustom::VideoPlayerCustom(
    std::unique_ptr<VideoDecoderCustom> decoder)
    : decoder_(std::move(decoder)) {}

VideoPlayerCustom::~VideoPlayerCustom() {
  if (decoder_) {
    decoder_->DestroyDecoder();
  }
}

std::unique_ptr<TextureInfo> VideoPlayerCustom::UpdateTexture(
    const int32_t index) {
  if (!asset_ || !decoder_ || !decoder_->IsValid()) {
    ANIMAX_LOGE("VideoPlayerCustom: No asset attached or no decoder valid.");
    return nullptr;
  }

  DCHECK(0 <= index && index < asset_->GetFrameCount());

  if (display_index_ == index) {
    return nullptr;
  }

  auto frame_info = DecodeAndCacheFrame(index);
  if (!frame_info) {
    ANIMAX_LOGW("VideoPlayerCustom: Failed to decode frame " +
                std::to_string(index));
    return nullptr;
  }

  display_index_ = index;
  PrepareNextFrame();
  return std::make_unique<TextureInfoFrameData>(frame_info, texture_target_);
}

void VideoPlayerCustom::SetTextureTarget(uint32_t texture_target) {
  texture_target_ = texture_target;
}

std::shared_ptr<YUVFrameInfo> VideoPlayerCustom::DecodeAndCacheFrame(
    const int32_t target_index) {
  if (!decoder_ || !decoder_->IsValid()) {
    return nullptr;
  }

  auto cache_frame_info = GetCachedFrame(target_index);
  if (cache_frame_info) {
    return cache_frame_info;
  }

  const int32_t cur_keyframe_index = asset_->GetPrevKeyFrame(decoded_index_);
  const int32_t new_keyframe_index = asset_->GetPrevKeyFrame(target_index);
  int32_t begin_index = (decoded_index_ + 1) % asset_->GetFrameCount();

  if (cur_keyframe_index != new_keyframe_index || target_index < begin_index) {
    begin_index = new_keyframe_index;
    if (cur_keyframe_index != new_keyframe_index) {
      ClearAllCachedFrames();
    }
  }

  for (int32_t index = begin_index; index <= target_index; ++index) {
    if (IsFrameCached(index)) {
      continue;
    }

    auto frame_data = asset_->GetFrameRawData(index);
    if (frame_data.empty()) {
      break;
    }

    auto decoded_frame =
        decoder_->DecodeFrameData(frame_data, GetReusableFrame());
    if (!decoded_frame) {
      break;
    }

    decoded_index_ = index;
    CacheFrame(index, std::move(decoded_frame));
  }

  return GetCachedFrame(target_index);
}

std::shared_ptr<YUVFrameInfo> VideoPlayerCustom::GetCachedFrame(
    const int32_t target_index) const {
  std::shared_lock lock(mutex_);
  auto cached_iter = frame_cache_.find(target_index);
  if (cached_iter != frame_cache_.end()) {
    return cached_iter->second;
  }
  return nullptr;
}

std::shared_ptr<YUVFrameInfo> VideoPlayerCustom::GetReusableFrame() {
  std::unique_lock lock(mutex_);
  if (reusable_frame_pool_.empty()) {
    return nullptr;
  }
  auto frame = reusable_frame_pool_.back();
  reusable_frame_pool_.pop_back();
  return frame;
}

bool VideoPlayerCustom::IsFrameCached(const int32_t frame_index) const {
  std::shared_lock lock(mutex_);
  return frame_cache_.find(frame_index) != frame_cache_.end();
}

void VideoPlayerCustom::CacheFrame(const int32_t target_index,
                                   std::shared_ptr<YUVFrameInfo> frameInfo) {
  std::unique_lock lock(mutex_);

  if (frame_cache_.size() >= kMaxCacheSize) {
    auto oldest = std::min_element(
        frame_cache_.begin(), frame_cache_.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });
    reusable_frame_pool_.push_back(oldest->second);
    frame_cache_.erase(oldest);
  }

  frame_cache_[target_index] = frameInfo;
}

void VideoPlayerCustom::ClearAllCachedFrames() {
  std::unique_lock lock(mutex_);
  frame_cache_.clear();
}

void VideoPlayerCustom::AttachAsset(std::shared_ptr<VideoAsset> asset) {
  if (!asset) {
    ANIMAX_LOGE("VideoPlayerCustom: Failed to attach asset with null asset.");
    return;
  }

  if (!decoder_) {
    ANIMAX_LOGE("VideoPlayerCustom: Failed to attach asset with null decoder.");
    return;
  }

  asset_ = asset;
  bool ret = decoder_->CreateDecoder(asset);
  if (!ret) {
    ANIMAX_LOGE("VideoPlayerCustom: Failed to create decoder");
    return;
  }

  std::vector<uint8_t> data = asset_->GetVideoParameterSets();
  if (!data.empty()) {
    decoder_->DecodeFrameData(data, nullptr);
  }

  PrepareNextFrame();
}

const std::array<float, 16>& VideoPlayerCustom::GetTransform() {
  return transform_;
}

void VideoPlayerCustom::PrepareNextFrame() {
  if (!asset_ || !decoder_ || !decoder_->IsValid()) {
    return;
  }

  int32_t next_index = (decoded_index_ + 1) % asset_->GetFrameCount();
  if (next_index != display_index_) {
    DecodeAndCacheFrame(next_index);
  }
}

}  // namespace animax
}  // namespace lynx
