// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_CUSTOM_YUV_FRAME_INFO_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_YUV_FRAME_INFO_H_

#include <cstdint>
#include <vector>

#include "include/base/macros.h"

namespace lynx {
namespace animax {

enum class YUVPixFmt : int64_t {
  kYUVPixFmtI400 = 0,  // Y only, 4:0:0
  kYUVPixFmtI420 = 1,  // YUV 4:2:0
  kYUVPixFmtI422 = 2,  // YUV 4:2:2
  kYUVPixFmtI444 = 3   // YUV 4:4:4
};

class ANIMAX_EXPORT YUVFrameInfo {
 public:
  static constexpr size_t kMaxChannels = 4;
  static constexpr size_t kYUVChannels = 3;

  YUVFrameInfo() = default;
  ~YUVFrameInfo() = default;

  /**
   * Get channel data for specified channel
   * Returns a point of the pixel data for the specified YUV channel
   * @param channel channel index (0=Y, 1=U, 2=V)
   * @return vector containing channel pixel data
   */
  const std::vector<uint8_t>* GetChannelData(uint32_t channel) const;

  /**
   * Get channel data width
   * Returns the actual data width for the specified channel
   * @param channel channel index (0=Y, 1=U, 2=V)
   * @return channel data width in pixels
   */
  uint32_t GetChannelDataWidth(uint32_t channel) const;

  /**
   * Get channel data height
   * Returns the actual data height for the specified channel
   * Height may be subsampled for chroma channels
   * @param channel channel index (0=Y, 1=U, 2=V)
   * @return channel data height in pixels
   */
  uint32_t GetChannelDataHeight(uint32_t channel) const;

  /**
   * Update channel data from external buffer by copying
   * data from external buffer to internal storage. The buffer
   * size calculation and resizing requirements:
   * - Original buffer size: line_size * height * vertical_subsampling_factor
   * - Target resizing size: line_width * height * vertical_subsampling_factor
   * @param channel channel index (0=Y, 1=U, 2=V)
   * @param buffer source buffer containing channel data
   * @return true if update successful, false otherwise
   */
  bool UpdateChannelData(uint32_t channel, const uint8_t* buffer);

  uint32_t GetWidth() const;
  uint32_t GetHeight() const;

  void SetLineSize(uint32_t channel, uint32_t line_size);
  void SetLineWidth(uint32_t channel, uint32_t line_width);
  void SetWidth(uint32_t width);
  void SetHeight(uint32_t height);
  void SetBitDepth(uint32_t bit_depth);
  void SetYUVPixFmt(YUVPixFmt pix_fmt);

 private:
  uint32_t line_size_[kMaxChannels] = {0};
  uint32_t line_width_[kMaxChannels] = {0};
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  uint32_t bit_depth_ = 8;
  YUVPixFmt pix_fmt_ = YUVPixFmt::kYUVPixFmtI420;

  float GetVerticalSubsamplingFactor(uint32_t channel) const;
  std::vector<std::vector<uint8_t>> data_ =
      std::vector<std::vector<uint8_t>>(kYUVChannels, std::vector<uint8_t>());
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_YUV_FRAME_INFO_H_
