// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/video/win/video_player_windows.h"

#include <memory>
#include <utility>

#include "src/resource/asset/video_asset.h"
#include "src/video/video_player.h"
#include "src/video/video_shader.h"

#if ENABLE_ANIMAX_VIDEO
#include "src/base/gl/gl_util.h"
#include "src/base/log/log.h"
#include "src/player/animax_ability.h"
#include "src/video/custom/ffmpeg/video_decoder_ffmpeg.h"
#include "src/video/custom/video_player_custom.h"
#include "src/video/win/video_shader_windows.h"
#endif

namespace lynx {
namespace animax {

#if ENABLE_ANIMAX_VIDEO

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    std::shared_ptr<AnimaXAbility> ability) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();
  auto player = std::make_unique<VideoPlayerCustom>(std::move(decoder));
  player->SetTextureTarget(GL_TEXTURE_2D);
  return player;
}

std::unique_ptr<VideoShader> VideoShader::Make(
    std::shared_ptr<AnimaXAbility> ability) {
  return std::make_unique<VideoShaderWindows>(std::move(ability));
}

#else  // !ENABLE_ANIMAX_VIDEO

// The FFmpeg backend is unavailable in this build (e.g. TTP cloud build
// where uniplayer is not delivered). Provide stub factories so animax_core
// still links; downstream code that hits these paths receives nullptr and
// is expected to no-op.
std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    std::shared_ptr<AnimaXAbility> ability) {
  return nullptr;
}

std::unique_ptr<VideoShader> VideoShader::Make(
    std::shared_ptr<AnimaXAbility> ability) {
  return nullptr;
}

std::shared_ptr<VideoAsset> VideoAsset::Make(VideoAssetModel model) {
  return nullptr;
}

#endif  // ENABLE_ANIMAX_VIDEO

}  // namespace animax
}  // namespace lynx
