// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_DECODER_CUSTOM_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_DECODER_CUSTOM_H_

#include <vector>

#include "include/player/animax_player.h"
#include "src/render/texture_info.h"
#include "src/resource/asset/video_asset.h"
#include "src/video/custom/yuv_frame_info.h"

namespace lynx {
namespace animax {

class VideoDecoderCustom {
 public:
  VideoDecoderCustom() = default;
  virtual ~VideoDecoderCustom() = default;

  /**
   * Check if this decoder is valid
   * @return true if this decoder is valid
   */
  virtual bool IsValid() = 0;

  /**
   * Initialize and create the decoder instance
   * @param asset Asset to decode.
   * @return true if this decoder created successfully
   */
  virtual bool CreateDecoder(std::shared_ptr<VideoAsset> asset) = 0;

  /**
   * Clean up and destroy the decoder instance
   * @return true if this decoder destroyed successfully
   */
  virtual bool DestroyDecoder() = 0;

  /**
   * Decode a single frame of video data
   *
   * This is the main decoding method that converts compressed video data into
   * a YUV frame. If a reusable frame is provided, the decoder will attempt to
   * reuse the existing frame object to avoid memory allocation overhead.
   * @param data Compressed video frame data to decode.
   * @param reusable_frame Reusable YUV frame information.
   * @return Decoded YUV frame information, or nullptr if
   * decoding fails.
   */
  virtual std::shared_ptr<YUVFrameInfo> DecodeFrameData(
      std::vector<uint8_t>& data,
      const std::shared_ptr<YUVFrameInfo>& reusable_frame) = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_DECODER_CUSTOM_H_
