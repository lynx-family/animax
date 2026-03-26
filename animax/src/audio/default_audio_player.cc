// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/audio_player.h"
#include "src/resource/asset/audio_asset.h"

namespace lynx {
namespace animax {

std::unique_ptr<AudioPlayer> AudioPlayer::MakeAudioPlayer(
    std::shared_ptr<AnimaXAbility> ability, std::shared_ptr<AudioAsset> asset) {
  return nullptr;
}

std::shared_ptr<AudioAsset> AudioAsset::Make(AudioAssetModel model) {
  return nullptr;
}

}  // namespace animax
}  // namespace lynx
