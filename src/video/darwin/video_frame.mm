// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/darwin/video_frame.h"

namespace lynx {
namespace animax {

VideoFrame::VideoFrame(CVPixelBufferRef pixel_buffer, CVMetalTextureCacheRef texture_cache_ref)
    : pixel_buffer_(pixel_buffer), texture_cache_ref_(texture_cache_ref) {}

VideoFrame::VideoFrame(VideoFrame &&other)
    : pixel_buffer_(other.pixel_buffer_),
      texture_cache_ref_(other.texture_cache_ref_),
      texture_ref_(other.texture_ref_),
      texture_(other.texture_) {
  other.pixel_buffer_ = nullptr;
  other.texture_cache_ref_ = nullptr;
  other.texture_ref_ = nullptr;
  other.texture_ = nullptr;
}

VideoFrame::~VideoFrame() {
  bool need_flush_cache = texture_ref_;
  if (texture_ref_) {
    CFRelease(texture_ref_);
    texture_ref_ = nullptr;
  }
  if (need_flush_cache && texture_cache_ref_) {
    CVMetalTextureCacheFlush(texture_cache_ref_, 0);
  }
  if (pixel_buffer_) {
    CVPixelBufferRelease(pixel_buffer_);
    pixel_buffer_ = nullptr;
  }
}

id<MTLTexture> VideoFrame::GetMTLTexture() {
  if (texture_) {
    return texture_;
  }
  if (!pixel_buffer_ || !texture_cache_ref_) {
    return nullptr;
  }

  // Create BGRA texture from pixel buffer
  size_t width = CVPixelBufferGetWidth(pixel_buffer_);
  size_t height = CVPixelBufferGetHeight(pixel_buffer_);
  MTLPixelFormat pixelFormat = MTLPixelFormatBGRA8Unorm;

  CVReturn status = CVMetalTextureCacheCreateTextureFromImage(
      kCFAllocatorDefault, texture_cache_ref_, pixel_buffer_, nullptr, pixelFormat, width, height,
      0, &texture_ref_);

  if (status == kCVReturnSuccess && texture_ref_) {
    texture_ = CVMetalTextureGetTexture(texture_ref_);
  }

  return texture_;
}

}  // namespace animax
}  // namespace lynx
