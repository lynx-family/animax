// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/web/video_player_web.h"

#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/render/texture_info_gl.h"
#include "src/video/web/video_asset_web.h"

namespace lynx {
namespace animax {

VideoPlayerWeb::VideoPlayerWeb(const AnimaXAbility *ability_ptr) {
  transform_.fill(0.f);
}

VideoPlayerWeb::~VideoPlayerWeb() {}

std::unique_ptr<TextureInfo> VideoPlayerWeb::UpdateTexture(
    const int32_t frame) {
  const int32_t to_frame = frame;
  DCHECK(0 <= to_frame && to_frame < asset_->GetFrameCount());
  if (current_frame_ == to_frame) {
    // don't need draw
    return nullptr;
  }
  // todo: impl
  return std::make_unique<TextureInfoGL>(
      video_texture_, asset_->GetVideoWidth(), asset_->GetVideoHeight(), 0);
}

const std::array<float, 16> &VideoPlayerWeb::GetTransform() {
  return transform_;
}

void VideoPlayerWeb::AttachAsset(VideoAsset *asset) {
  DCHECK(asset);
  asset_ = static_cast<VideoAssetWeb *>(asset);
}

void VideoPlayerWeb::NotifyErrorEvent(const std::string &err_msg) {
  if (listener_) {
    listener_->OnVideoPlayerError(err_msg);
  }
}

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    const AnimaXAbility *ability_ptr) {
  return std::unique_ptr<VideoPlayerWeb>(new VideoPlayerWeb(ability_ptr));
}

}  // namespace animax
}  // namespace lynx
