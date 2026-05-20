// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_VIDEO_PLAYER_PROVIDER_H_
#define ANIMAX_SRC_VIDEO_VIDEO_PLAYER_PROVIDER_H_

#include "src/resource/asset/video_asset.h"
#include "src/video/video_player.h"
#include "src/video/video_shader.h"

namespace lynx {
namespace animax {

class VideoPlayerProvider {
 public:
  virtual ~VideoPlayerProvider() = default;

  /**
   * Creates video player instance based on the provided player and asset.
   * @param ability The ability to check support for.
   * @return A unique_ptr to the created VideoPlayer instance, or nullptr if
   * creation fails.
   */
  virtual std::unique_ptr<VideoPlayer> CreateVideoPlayer(
      std::shared_ptr<AnimaXAbility> ability) = 0;

  /**
   * Creates video shader instance based on the provided player and asset.
   * @return A unique_ptr to the created VideoShader instance, or nullptr if
   * creation fails.
   */
  virtual std::unique_ptr<VideoShader> CreateVideoShader(
      std::shared_ptr<AnimaXAbility> ability) = 0;

  /**
   * Checks if this provider supports the given ability and video asset.
   * @param ability The ability to check support for.
   * @param asset The video asset to check support for.
   * @return true if this provider supports the ability and asset, false
   * otherwise.
   */
  virtual bool IsSupported(AnimaXAbility* ability, VideoAsset* asset) = 0;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_VIDEO_PLAYER_PROVIDER_H_
