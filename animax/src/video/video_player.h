// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_VIDEO_PLAYER_H_
#define ANIMAX_SRC_VIDEO_VIDEO_PLAYER_H_

#include <array>
#include <memory>

#include "src/base/log/log.h"
#include "src/player/animax_ability.h"
#include "src/render/texture_info.h"
#include "src/video/video_player_listener.h"

namespace lynx {
namespace animax {

class VideoAsset;
class VideoPlayer {
 public:
  virtual ~VideoPlayer() {
    ANIMAX_LOGI("Video player is destroyed, this: " << this);
  }
  static std::unique_ptr<VideoPlayer> MakeVideoPlayer(
      const AnimaXAbility *ability_ptr);

  /**
   * Let player decode specific frame, and return that texture
   * @param frame a value between 0 and (frameCount - 1)
   * @return      a texture containing necessary info
   */
  virtual std::unique_ptr<TextureInfo> UpdateTexture(const int32_t frame) = 0;
  /**
   * Get transform of output texture of UpdateTexture
   * This method should be called after UpdateTexture. The return value is used
   * as transform matrix in fragment shader
   * @return transform matrix used in fragment shader
   */
  virtual const std::array<float, 16> &GetTransform() = 0;
  /**
   * Attach an asset to player
   * @param asset an asset to be played
   */
  virtual void AttachAsset(std::shared_ptr<VideoAsset> asset) = 0;
  /**
   * Set video player listener
   * make sure listener is valid in the lifecycle of video player.
   * @param listener a video player listener
   */
  void SetListener(VideoPlayerListener *listener) { listener_ = listener; }

 protected:
  VideoPlayer() { ANIMAX_LOGI("Video player is created, this: " << this); }

  VideoPlayerListener *listener_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_VIDEO_PLAYER_H_
