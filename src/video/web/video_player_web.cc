// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <memory>

#include "src/video/custom/video_player_custom.h"
#include "src/video/custom/video_shader_yuv.h"
#include "src/video/video_player.h"
#include "src/video/video_shader.h"
#include "src/video/web/video_decoder_web.h"

namespace lynx {
namespace animax {

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    std::shared_ptr<AnimaXAbility> ability) {
  auto decoder = std::make_unique<VideoDecoderWeb>();
  auto player = std::make_unique<VideoPlayerCustom>(std::move(decoder));
  player->SetTextureTarget(GL_TEXTURE_2D);
  return player;
}

std::unique_ptr<VideoShader> VideoShader::Make(
    std::shared_ptr<AnimaXAbility> ability) {
  auto shader = std::make_unique<VideoShaderYUV>();
  shader->SetTextureFormat(GL_LUMINANCE);
  return shader;
}

}  // namespace animax
}  // namespace lynx
