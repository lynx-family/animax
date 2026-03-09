// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_PLAYER_CUSTOM_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_PLAYER_CUSTOM_H_

#include <shared_mutex>

#include "include/base/macros.h"
#include "src/base/gl/gl_include.h"
#include "src/player/animax_ability.h"
#include "src/video/custom/video_decoder_custom.h"
#include "src/video/video_player.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT VideoPlayerCustom : public VideoPlayer {
 public:
  explicit VideoPlayerCustom(std::unique_ptr<VideoDecoderCustom> decoder);
  ~VideoPlayerCustom() override;

  std::unique_ptr<TextureInfo> UpdateTexture(const int32_t frame) override;
  const std::array<float, 16>& GetTransform() override;
  void AttachAsset(std::shared_ptr<VideoAsset> asset) override;
  void SetTextureTarget(uint32_t texture_target);

 private:
  uint32_t texture_target_ = 0;
  // Frame managment
  void ClearAllCachedFrames();
  void CacheFrame(const int32_t target_index,
                  std::shared_ptr<YUVFrameInfo> frameInfo);
  bool IsFrameCached(const int32_t frame_index) const;
  std::shared_ptr<YUVFrameInfo> GetCachedFrame(
      const int32_t target_index) const;
  std::shared_ptr<YUVFrameInfo> GetReusableFrame();
  static constexpr size_t kMaxCacheSize = 10;

  // Decode ability
  std::shared_ptr<YUVFrameInfo> DecodeAndCacheFrame(const int32_t target_index);
  void PrepareNextFrame();

  std::unordered_map<int32_t, std::shared_ptr<YUVFrameInfo>> frame_cache_;
  std::vector<std::shared_ptr<YUVFrameInfo>> reusable_frame_pool_;
  std::unique_ptr<VideoDecoderCustom> decoder_ = nullptr;
  std::array<float, 16> transform_{};
  std::shared_ptr<VideoAsset> asset_;
  mutable std::shared_mutex mutex_;
  static constexpr int32_t kCurrentFrameInvalid = -1;
  int32_t display_index_ = kCurrentFrameInvalid;
  int32_t decoded_index_ = kCurrentFrameInvalid;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_PLAYER_CUSTOM_H_
