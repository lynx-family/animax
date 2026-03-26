// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_CUSTOM_BYTEVC1_ANDROID_VIDEO_PLAYER_PROVIDER_BYTEVC1_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_BYTEVC1_ANDROID_VIDEO_PLAYER_PROVIDER_BYTEVC1_H_

#include "src/video/video_player_provider.h"

namespace lynx {
namespace animax {

class VideoPlayerProviderByteVc1 : public VideoPlayerProvider {
 public:
  std::unique_ptr<VideoPlayer> CreateVideoPlayer(
      const AnimaXAbility *ability) override;

  std::unique_ptr<VideoShader> CreateVideoShader(
      const AnimaXAbility *ability) override;

  bool IsSupported(const AnimaXAbility *ability,
                   std::shared_ptr<VideoAsset> asset) override;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_BYTEVC1_ANDROID_VIDEO_PLAYER_PROVIDER_BYTEVC1_H_
