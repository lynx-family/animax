// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/darwin/video_frame_cache.h"

#include <algorithm>

namespace lynx {
namespace animax {

VideoFrameCache::VideoFrameCache() {
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  CVMetalTextureCacheCreate(kCFAllocatorDefault, NULL, device, NULL, &texture_cache_ref_);
}

VideoFrameCache::VideoFrameCache(VideoFrameCache &&other)
    : frames_(std::move(other.frames_)), texture_cache_ref_(other.texture_cache_ref_) {
  other.texture_cache_ref_ = 0;
}

VideoFrameCache::~VideoFrameCache() {
  if (texture_cache_ref_) {
    CVMetalTextureCacheFlush(texture_cache_ref_, 0);
    CFRelease(texture_cache_ref_);
    texture_cache_ref_ = nullptr;
  }
}

void VideoFrameCache::Add(int32_t presentation_index, CVPixelBufferRef pixel_buffer) {
  frames_[presentation_index] = std::make_unique<VideoFrame>(pixel_buffer, texture_cache_ref_);
}

bool VideoFrameCache::Contains(int32_t presentation_index) const {
  return frames_.count(presentation_index) > 0;
}

std::unique_ptr<VideoFrame> VideoFrameCache::Evict(int32_t presentation_index) {
  auto it = std::find_if(frames_.begin(), frames_.end(), [presentation_index](auto &item) {
    return item.first == presentation_index;
  });
  if (it == frames_.end()) {
    return nullptr;
  }
  auto frame = std::move(it->second);
  frames_.erase(it);
  return frame;
}

void VideoFrameCache::ClearCache(int32_t from_frame, int32_t to_frame) {
  auto it = frames_.begin();
  while (frames_.end() != it) {
    if (it->first >= from_frame && it->first < to_frame) {
      it = frames_.erase(it);
    } else {
      ++it;
    }
  }
}

void VideoFrameCache::ClearAll() { frames_.clear(); }

}  // namespace animax
}  // namespace lynx
