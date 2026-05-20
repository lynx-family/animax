// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_VIDEO_PLAYER_PROVIDER_DEFAULT_H_
#define ANIMAX_SRC_VIDEO_VIDEO_PLAYER_PROVIDER_DEFAULT_H_

#include <utility>

#include "src/video/video_player_provider.h"

namespace lynx {
namespace animax {

class VideoPlayerProviderDefault : public VideoPlayerProvider {
 public:
  std::unique_ptr<VideoPlayer> CreateVideoPlayer(
      std::shared_ptr<AnimaXAbility> ability) override {
    return VideoPlayer::MakeVideoPlayer(std::move(ability));
  }

  std::unique_ptr<VideoShader> CreateVideoShader(
      std::shared_ptr<AnimaXAbility> ability) override {
    return VideoShader::Make(std::move(ability));
  }

  bool IsSupported(AnimaXAbility* ability, VideoAsset* asset) override {
    return true;
  }
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_VIDEO_PLAYER_PROVIDER_DEFAULT_H_
