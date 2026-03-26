// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_WEB_VIDEO_PLAYER_WEB_H_
#define ANIMAX_SRC_VIDEO_WEB_VIDEO_PLAYER_WEB_H_

#include "src/player/animax_ability.h"
#include "src/video/video_player.h"

namespace lynx {
namespace animax {

class VideoAssetWeb;
class VideoPlayerWeb : public VideoPlayer {
 public:
  VideoPlayerWeb(const AnimaXAbility *ability_ptr);
  ~VideoPlayerWeb() override;

  std::unique_ptr<TextureInfo> UpdateTexture(const int32_t frame) override;
  const std::array<float, 16> &GetTransform() override;
  void AttachAsset(std::shared_ptr<VideoAsset> asset) override;

  void NotifyErrorEvent(const std::string &err_msg);

 private:
  std::shared_ptr<VideoAssetWeb> asset_;
  uint32_t video_texture_ = 0;
  std::array<float, 16> transform_{};
  int32_t current_frame_ = -1;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_WEB_VIDEO_PLAYER_WEB_H_
