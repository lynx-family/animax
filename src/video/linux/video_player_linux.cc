// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/linux/video_player_linux.h"

namespace lynx {
namespace animax {

// Linux does not provide a native AnimaX video backend yet. Keep the platform
// factories available so non-video compositions can still use AnimaX.
std::unique_ptr<VideoShader> VideoShader::Make(std::shared_ptr<AnimaXAbility>) {
  return nullptr;
}

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    std::shared_ptr<AnimaXAbility>) {
  return nullptr;
}

std::shared_ptr<VideoAsset> VideoAsset::Make(VideoAssetModel) {
  return nullptr;
}

}  // namespace animax
}  // namespace lynx
