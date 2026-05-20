// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/video/win/video_shader_windows.h"

#include <memory>
#include <utility>

#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/player/win/animax_ability_windows.h"
#include "src/render/texture_info_frame_data.h"
#include "src/video/win/video_player_windows.h"

namespace lynx {
namespace animax {

VideoShaderWindows::VideoShaderWindows(std::shared_ptr<AnimaXAbility> ability)
    : VideoShaderYUV() {
  ANIMAX_LOGI("[VideoShader] Initializing YUV video shader");
  ability_windows_ =
      std::static_pointer_cast<AnimaXAbilityWindows>(std::move(ability));
  DCHECK(ability_windows_);
  raster_runner_ = ability_windows_->GetRasterRunner();
  DCHECK(raster_runner_);
  weak_player_ = ability_windows_->GetAnimaXPlayer();
}

void VideoShaderWindows::PostToRasterThread(base::MoveOnlyClosure<void> task) {
  if (!raster_runner_ || !ability_windows_) {
    return;
  }

  // For Windows, GL-related operations need to be executed on the raster
  // thread.
  // Note: 'this' pointer is captured by reference and will be held by the
  // player.
  fml::TaskRunner::RunNowOrPostTask(
      raster_runner_,
      [this, task = std::move(task), weak_player = weak_player_]() mutable {
        auto player = weak_player.lock();
        if (!player) {
          return;
        }
        if (!raster_runner_) {
          return;
        }
        task();
      });
}

VideoShaderWindows::~VideoShaderWindows() {
  if (!raster_runner_) {
    ANIMAX_LOGE("Destroy failed: raster runner is null");
    return;
  }

  auto program = std::exchange(program_, 0);
  auto fbo = std::exchange(fbo_, 0);
  auto texture = std::exchange(texture_, 0);
  auto textures = std::move(textures_);

  if (program == 0 && fbo == 0 && texture == 0 && textures.empty()) {
    return;
  }

  if (!raster_runner_) {
    ANIMAX_LOGE("Destroy failed: raster runner is null");
    return;
  }

  PostToRasterThread([program, fbo, texture, textures = std::move(textures)]() {
    if (program != 0) {
      glDeleteProgram(program);
    }
    if (fbo != 0) {
      glDeleteFramebuffers(1, &fbo);
    }
    if (texture != 0) {
      glDeleteTextures(1, &texture);
    }
    if (!textures.empty()) {
      glDeleteTextures(textures.size(), textures.data());
    }
  });
}

void VideoShaderWindows::Init(int32_t w, int32_t h,
                              const std::array<float, 4> &rgb_frame,
                              const std::array<float, 4> &a_frame) {
  PostToRasterThread([this, w, h, rgb_frame, a_frame]() {
    VideoShaderYUV::Init(w, h, rgb_frame, a_frame);
  });
}

void VideoShaderWindows::Draw(std::unique_ptr<TextureInfo> texture_info,
                              const std::array<float, 16> &transform) {
  if (!Valid() || !texture_info) {
    return;
  }
  PostToRasterThread(
      [this, texture_info = std::move(texture_info), transform]() mutable {
        if (!Valid()) {
          return;
        }
        VideoShaderYUV::Draw(std::move(texture_info), transform);
      });
}

}  // namespace animax
}  // namespace lynx
