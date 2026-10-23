// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/linux/video_player_linux.h"

#include "src/base/gl/gl_include.h"
#include "src/video/custom/ffmpeg/video_decoder_ffmpeg.h"
#include "src/video/custom/video_player_custom.h"

namespace lynx {
namespace animax {

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    std::shared_ptr<AnimaXAbility> ability) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();
  auto player = std::make_unique<VideoPlayerCustom>(std::move(decoder));
  player->SetTextureTarget(GL_TEXTURE_2D);
  return player;
}

}  // namespace animax
}  // namespace lynx
