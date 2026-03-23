// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/yuv_frame_info.h"

#include <cmath>
#include <cstring>

namespace lynx {
namespace animax {

uint32_t YUVFrameInfo::GetWidth() const { return width_; }

uint32_t YUVFrameInfo::GetHeight() const { return height_; }

void YUVFrameInfo::SetLineSize(uint32_t channel, uint32_t line_size) {
  if (channel >= 0 && channel < kYUVChannels) {
    line_size_[channel] = line_size;
  }
}

void YUVFrameInfo::SetWidth(uint32_t width) { width_ = width; }

void YUVFrameInfo::SetHeight(uint32_t height) { height_ = height; }

void YUVFrameInfo::SetBitDepth(uint32_t bit_depth) { bit_depth_ = bit_depth; }

void YUVFrameInfo::SetYUVPixFmt(YUVPixFmt pix_fmt) { pix_fmt_ = pix_fmt; }

const std::vector<uint8_t>* YUVFrameInfo::GetChannelData(
    uint32_t channel) const {
  if (channel < 0 || channel >= kYUVChannels) {
    return nullptr;
  }
  return &data_[channel];
}

uint32_t YUVFrameInfo::GetChannelDataWidth(uint32_t channel) const {
  if (channel < 0 || channel >= kYUVChannels) {
    return 0;
  }
  return std::ceil(width_ * GetWidthSubsamplingFactor(channel));
}

uint32_t YUVFrameInfo::GetChannelDataHeight(uint32_t channel) const {
  if (channel < 0 || channel >= kYUVChannels) {
    return 0;
  }
  return std::ceil(height_ * GetHeightSubsamplingFactor(channel));
}

bool YUVFrameInfo::UpdateChannelData(uint32_t channel, const uint8_t* buffer) {
  if (channel < 0 || channel >= kYUVChannels || buffer == nullptr) {
    return false;
  }

  const uint32_t sampling_height = GetChannelDataHeight(channel);
  const uint32_t src_line_width = GetChannelDataWidth(channel);
  const uint32_t src_line_size = line_size_[channel];

  if (sampling_height <= 0 || src_line_width == 0 || src_line_size == 0) {
    data_[channel].clear();
    return false;
  }

  const int pixel_bytes = (bit_depth_ > 8) ? 2 : 1;
  const uint32_t dst_line_size = src_line_width * pixel_bytes;
  if (dst_line_size > src_line_size) {
    data_[channel].clear();
    return false;
  }

  const size_t required_size =
      static_cast<size_t>(dst_line_size) * sampling_height;
  if (data_[channel].capacity() < required_size) {
    data_[channel].reserve(required_size);
  }
  data_[channel].resize(required_size);

  const uint8_t* src_pixels = buffer;
  uint8_t* dst_pixels = data_[channel].data();
  for (int row = 0; row < sampling_height; ++row) {
    std::memcpy(dst_pixels, src_pixels, dst_line_size);
    dst_pixels += dst_line_size;
    src_pixels += src_line_size;
  }

  return true;
}

float YUVFrameInfo::GetHeightSubsamplingFactor(uint32_t channel) const {
  if (channel < 0 || channel >= kYUVChannels) {
    return 0.0f;
  }

  if (channel == 0) {
    return 1.0f;  // Y plane has no subsampling
  }

  switch (pix_fmt_) {
    case YUVPixFmt::kYUVPixFmtI400:
      return 1.0f;
    case YUVPixFmt::kYUVPixFmtI420:
      return 0.5f;
    case YUVPixFmt::kYUVPixFmtI422:
      return 1.0f;
    case YUVPixFmt::kYUVPixFmtI444:
      return 1.0f;
    default:
      return 1.0f;
  }
}

float YUVFrameInfo::GetWidthSubsamplingFactor(uint32_t channel) const {
  if (channel < 0 || channel >= kYUVChannels) {
    return 0.0f;
  }

  if (channel == 0) {
    return 1.0f;  // Y plane has no subsampling
  }

  switch (pix_fmt_) {
    case YUVPixFmt::kYUVPixFmtI400:
      return 1.0f;
    case YUVPixFmt::kYUVPixFmtI420:
      return 0.5f;
    case YUVPixFmt::kYUVPixFmtI422:
      return 0.5f;
    case YUVPixFmt::kYUVPixFmtI444:
      return 1.0f;
    default:
      return 1.0f;
  }
}

}  // namespace animax
}  // namespace lynx
