// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/video_player_custom.h"

#include <algorithm>

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

  if (target_index < 0 ||
      static_cast<size_t>(target_index) >= display_pts_.size()) {
    return nullptr;
  }

  const auto target_pts = display_pts_[target_index];
  auto cache_frame_info = GetCachedFrame(target_pts);
  if (cache_frame_info) {
    return cache_frame_info;
  }

  if (target_pts <= decoded_pts_ || target_index > display_index_ + 1) {
    const auto start_packet = FindDecodeStartPacket(target_pts);
    if ((target_pts <= decoded_pts_ || start_packet > next_packet_index_) &&
        !SeekToPacket(start_packet)) {
      return nullptr;
    }
  }

  auto frame = GetReusableFrame();
  while (decoded_pts_ < target_pts) {
    switch (decoder_->ReceiveFrame(frame)) {
      case VideoDecodeStatus::kFrameReady:
        decoded_pts_ = frame->GetPts();
        if (decoded_pts_ == YUVFrameInfo::kNoPts) {
          ANIMAX_LOGE("VideoPlayerCustom: Decoded frame has no timestamp.");
          return nullptr;
        }
        CacheFrame(frame);
        if (decoded_pts_ < target_pts) {
          frame = GetReusableFrame();
        }
        break;
      case VideoDecodeStatus::kNeedInput: {
        if (input_ended_) {
          ANIMAX_LOGE("VideoPlayerCustom: Decoder requested input after EOS.");
          return nullptr;
        }
        std::vector<uint8_t> data;
        int64_t pts = YUVFrameInfo::kNoPts;
        if (next_packet_index_ < asset_->GetFrameCount()) {
          data = asset_->GetFrameRawData(next_packet_index_);
          if (data.empty()) {
            return nullptr;
          }
          pts = packet_pts_[next_packet_index_];
        }
        if (decoder_->SendPacket(data, pts) != VideoSendStatus::kSuccess) {
          return nullptr;
        }
        if (data.empty()) {
          input_ended_ = true;
        } else {
          ++next_packet_index_;
        }
        break;
      }
      case VideoDecodeStatus::kEndOfStream:
      case VideoDecodeStatus::kError:
        return nullptr;
    }
  }

  return GetCachedFrame(target_pts);
}

std::shared_ptr<YUVFrameInfo> VideoPlayerCustom::GetCachedFrame(
    int64_t pts) const {
  std::shared_lock lock(mutex_);
  auto cached_iter = frame_cache_.find(pts);
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

void VideoPlayerCustom::CacheFrame(
    const std::shared_ptr<YUVFrameInfo>& frameInfo) {
  std::unique_lock lock(mutex_);

  if (frame_cache_.size() >= kMaxCacheSize) {
    auto oldest = std::min_element(
        frame_cache_.begin(), frame_cache_.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });
    reusable_frame_pool_.push_back(oldest->second);
    frame_cache_.erase(oldest);
  }

  frame_cache_[frameInfo->GetPts()] = frameInfo;
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
  display_index_ = kCurrentFrameInvalid;
  decoder_->DestroyDecoder();
  ClearAllCachedFrames();
  decoded_pts_ = YUVFrameInfo::kNoPts;
  next_packet_index_ = 0;
  input_ended_ = false;

  packet_pts_ = asset_->GetFrameTimestamps();
  display_pts_ = packet_pts_;
  std::sort(display_pts_.begin(), display_pts_.end());
  if (display_pts_.empty() ||
      display_pts_.size() != static_cast<size_t>(asset_->GetFrameCount()) ||
      display_pts_.front() == YUVFrameInfo::kNoPts ||
      std::adjacent_find(display_pts_.begin(), display_pts_.end()) !=
          display_pts_.end()) {
    ANIMAX_LOGE("VideoPlayerCustom: Invalid frame timestamps.");
    return;
  }

  if (!decoder_->CreateDecoder(asset_)) {
    ANIMAX_LOGE("VideoPlayerCustom: Failed to create decoder");
    return;
  }

  std::vector<uint8_t> data = asset_->GetVideoParameterSets();
  if (!data.empty() && decoder_->SendPacket(data, YUVFrameInfo::kNoPts) !=
                           VideoSendStatus::kSuccess) {
    decoder_->DestroyDecoder();
    return;
  }

  PrepareNextFrame();
}

int32_t VideoPlayerCustom::FindDecodeStartPacket(int64_t target_pts) const {
  const auto packet =
      std::find(packet_pts_.begin(), packet_pts_.end(), target_pts);
  int32_t key_packet = asset_->GetPrevKeyFrame(
      static_cast<int32_t>(packet - packet_pts_.begin()));
  // An open GOP can put its keyframe before B packets displayed before it.
  // Those B frames still need the preceding GOP's references.
  while (key_packet > 0 && packet_pts_[key_packet] > target_pts) {
    key_packet = asset_->GetPrevKeyFrame(key_packet - 1);
  }
  return std::max(key_packet, 0);
}

bool VideoPlayerCustom::SeekToPacket(int32_t packet_index) {
  if (!decoder_->Flush()) {
    return false;
  }
  ClearAllCachedFrames();
  decoded_pts_ = YUVFrameInfo::kNoPts;
  next_packet_index_ = packet_index;
  input_ended_ = false;
  return true;
}

const std::array<float, 16>& VideoPlayerCustom::GetTransform() {
  return transform_;
}

void VideoPlayerCustom::PrepareNextFrame() {
  if (!asset_ || !decoder_ || !decoder_->IsValid()) {
    return;
  }

  int32_t next_index = display_index_ + 1;
  if (next_index < asset_->GetFrameCount() && next_index != display_index_) {
    DecodeAndCacheFrame(next_index);
  }
}

}  // namespace animax
}  // namespace lynx
