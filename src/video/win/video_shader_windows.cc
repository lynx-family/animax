// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/video/win/video_shader_windows.h"

#include <memory>
#include <mutex>
#include <utility>

#include "src/base/log/log.h"
#include "src/player/win/animax_ability_windows.h"
#include "src/render/image.h"
#include "src/video/custom/video_shader_yuv.h"

namespace lynx {
namespace animax {

namespace {

class RasterVideoShader : public VideoShaderYUV {
 public:
  explicit RasterVideoShader(fml::RefPtr<fml::TaskRunner> raster_runner)
      : raster_runner_(std::move(raster_runner)) {}

  ~RasterVideoShader() override {
    // The shader is destroyed on its owner thread. Transfer only GL handles
    // to the raster thread so base destructors cannot delete them here.
    auto program = std::exchange(program_, 0);
    auto fbo = std::exchange(fbo_, 0);
    auto texture = std::exchange(texture_, 0);
    std::vector<GLuint> textures;
    textures.swap(textures_);
    if (program == 0 && fbo == 0 && texture == 0 && textures.empty()) {
      return;
    }
    fml::TaskRunner::RunNowOrPostTask(
        raster_runner_,
        [program, fbo, texture, textures = std::move(textures)]() {
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

 private:
  fml::RefPtr<fml::TaskRunner> raster_runner_;
};

}  // namespace

struct VideoShaderWindows::RasterState {
  explicit RasterState(fml::RefPtr<fml::TaskRunner> raster_runner)
      : shader(std::make_unique<RasterVideoShader>(std::move(raster_runner))) {}

  std::mutex mutex;
  std::unique_ptr<VideoShaderYUV> shader;
};

VideoShaderWindows::VideoShaderWindows(std::shared_ptr<AnimaXAbility> ability)
    : VideoShader() {
  ANIMAX_LOGI("[VideoShader] Initializing YUV video shader");
  auto ability_windows =
      std::static_pointer_cast<AnimaXAbilityWindows>(std::move(ability));
  DCHECK(ability_windows);
  raster_runner_ = ability_windows->GetRasterRunner();
  DCHECK(raster_runner_);
  weak_player_ = ability_windows->GetAnimaXPlayer();
  raster_state_ = std::make_shared<RasterState>(raster_runner_);
}

void VideoShaderWindows::PostToRasterThread(
    base::MoveOnlyClosure<void, VideoShaderYUV &> task) {
  if (!raster_runner_) {
    return;
  }

  // A live player does not guarantee a live shader: its layer can replace the
  // shader independently. Keep state alive and serialize use with teardown.
  fml::TaskRunner::RunNowOrPostTask(
      raster_runner_, [state = raster_state_, task = std::move(task),
                       weak_player = weak_player_]() mutable {
        auto player = weak_player.lock();
        if (!player) {
          return;
        }
        std::lock_guard<std::mutex> lock(state->mutex);
        if (!state->shader) {
          return;
        }
        task(*state->shader);
      });
}

VideoShaderWindows::~VideoShaderWindows() {
  // Wait only for an in-flight shader operation, never for a queued task.
  // Pending tasks retain the state and skip work after the shader is reset.
  std::lock_guard<std::mutex> lock(raster_state_->mutex);
  raster_state_->shader.reset();
}

bool VideoShaderWindows::Valid() {
  std::lock_guard<std::mutex> lock(raster_state_->mutex);
  return raster_state_->shader && raster_state_->shader->Valid();
}

void VideoShaderWindows::Init(int32_t w, int32_t h,
                              const std::array<float, 4> &rgb_frame,
                              const std::array<float, 4> &a_frame) {
  PostToRasterThread([w, h, rgb_frame, a_frame](VideoShaderYUV &shader) {
    shader.Init(w, h, rgb_frame, a_frame);
  });
}

void VideoShaderWindows::Draw(std::unique_ptr<TextureInfo> texture_info,
                              const std::array<float, 16> &transform) {
  if (!texture_info) {
    return;
  }
  PostToRasterThread([texture_info = std::move(texture_info),
                      transform](VideoShaderYUV &shader) mutable {
    shader.Draw(std::move(texture_info), transform);
  });
}

std::unique_ptr<Image> VideoShaderWindows::GetOutputImage(
    RealContext *context) {
  std::lock_guard<std::mutex> lock(raster_state_->mutex);
  return raster_state_->shader ? raster_state_->shader->GetOutputImage(context)
                               : nullptr;
}

}  // namespace animax
}  // namespace lynx
