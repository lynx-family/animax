// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_DARWIN_VIDEO_PLAYER_DARWIN_H_
#define ANIMAX_SRC_VIDEO_DARWIN_VIDEO_PLAYER_DARWIN_H_

#import <AVFoundation/AVFoundation.h>
#import <VideoToolbox/VideoToolbox.h>

#include <shared_mutex>
#include <string>
#include <unordered_set>

#include "src/render/texture_info.h"
#include "src/video/darwin/pending_frame_set.h"
#include "src/video/darwin/video_asset_darwin.h"
#include "src/video/video_player.h"

namespace lynx {
namespace animax {
class VideoAssetDarwin;
class VideoPlayerErrorReporter;
class VideoFrame;
class VideoFrameCache;

class VideoPlayerDarwin : public VideoPlayer {
 public:
  VideoPlayerDarwin();
  ~VideoPlayerDarwin() override;

  std::unique_ptr<TextureInfo> UpdateTexture(const int32_t frame) override;
  const std::array<float, 16> &GetTransform() override;
  void AttachAsset(std::shared_ptr<VideoAsset> asset) override;
  void UpdateOutputFrameSize(const int32_t width,
                             const int32_t height) override;

 private:
  friend VideoPlayerErrorReporter;
  void ResetSession();
  void ReleaseSession();
  PendingFrameSet::FlushResult FlushPendingFrameSet(
      const int32_t required_presentation_index, const bool sync);
  CMSampleBufferRef PrepareFrameData(const FrameInfo &frame_info);
  bool DecodeFrameData(CMSampleBufferRef sample_buffer,
                       const int32_t presentation_index,
                       const bool need_output_frame);
  bool IsRestartableDecodeError(OSStatus status) const;
  void HandleDecodeError(OSStatus status, const std::string &message_prefix);
  int32_t DecodeFrame(const int32_t decode_index,
                      const int32_t target_presentation_index);
  void PrepareNextFrame(const int32_t target_presentation_index);
  int32_t GetNextFrameToDecode(int32_t presentation_frame);
  VideoFrameCache *GetFrameCache();
  void MoveFrameFromCache(int32_t presentation_frame);

  std::shared_ptr<VideoAssetDarwin> asset_ = nullptr;
  std::array<float, 16> transform_;
  std::unique_ptr<VideoPlayerErrorReporter> error_reporter_ = nullptr;

  VTDecompressionSessionRef session_ = nullptr;
  bool session_valid_ = false;
  bool should_restart_ = false;
  static constexpr int32_t CURRENT_FRAME_INVALID = -1;
  int32_t current_decoded_frame_ = CURRENT_FRAME_INVALID;

  int32_t current_presentation_frame_ = CURRENT_FRAME_INVALID;
  std::unique_ptr<VideoFrameCache> frame_cache_ = nullptr;
  std::unique_ptr<VideoFrame> current_frame_ = nullptr;

  std::shared_ptr<PendingFrameSet> pending_frame_set_ = nullptr;

  static constexpr int32_t kMaxRetryCount = 5;
  int32_t decoder_retry_count_ = 0;
  mutable std::shared_mutex mutex_;

  int32_t output_width_ = 0;
  int32_t output_height_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_DARWIN_VIDEO_PLAYER_DARWIN_H_
