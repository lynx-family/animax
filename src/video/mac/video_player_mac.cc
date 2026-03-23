// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/video/mac/video_player_mac.h"

#include <memory>

#include "src/resource/asset/video_asset.h"
#include "src/video/video_player.h"
#include "src/video/video_shader.h"

namespace lynx {
namespace animax {

std::unique_ptr<VideoShader> VideoShader::Make(
    const AnimaXAbility* ability_ptr) {
  return nullptr;
}

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    const AnimaXAbility* ability_ptr) {
  return nullptr;
}

std::shared_ptr<VideoAsset> VideoAsset::Make(VideoAssetModel model) {
  return nullptr;
}

}  // namespace animax
}  // namespace lynx
