// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_DECODER_CUSTOM_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_DECODER_CUSTOM_H_

#include <vector>

#include "include/player/animax_player.h"
#include "src/render/texture_info.h"
#include "src/resource/asset/video_asset.h"
#include "src/video/custom/video_decode_status.h"
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

  // Submit a packet after ReceiveFrame returns kNeedInput, or immediately
  // after creation for codec parameter sets. Empty data ends input; send it
  // once, then receive the remaining frames. A successful packet must not be
  // submitted again. Calls to SendPacket and ReceiveFrame must be serialized.
  // pts is in microseconds; parameter sets and EOS use YUVFrameInfo::kNoPts.
  virtual VideoSendStatus SendPacket(std::vector<uint8_t>& data,
                                     int64_t pts) = 0;

  // Receive the next frame in display order. A non-null frame is reused;
  // its contents are valid only when kFrameReady is returned.
  virtual VideoDecodeStatus ReceiveFrame(
      std::shared_ptr<YUVFrameInfo>& frame) = 0;

  // Discard buffered input/output for seek, retaining the decoder instance
  // and codec configuration. The next packet must start at a keyframe.
  virtual bool Flush() = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_DECODER_CUSTOM_H_
