// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/linux/video_shader_linux.h"

#include <atomic>

#include "skity/gpu/gpu_context_gl.hpp"
#include "skity/graphic/image.hpp"
#include "src/base/thread/thread_assert.h"
#include "src/render/image.h"
#include "src/render/skity_promise_texture.h"
#include "src/video/custom/video_shader_yuv.h"
#include "src/video/linux/animax_ability_linux.h"

namespace lynx {
namespace animax {
namespace {
class VideoImage final : public Image {
 public:
  explicit VideoImage(std::shared_ptr<skity::Image> image) {
    image_ = std::move(image);
  }
};
}  // namespace

class VideoShaderLinux::State : public VideoShaderYUV {
 public:
  std::atomic<bool> failed{false};

  bool FramebufferComplete() {
    GLint previous;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    const bool complete =
        glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, previous);
    return complete;
  }

  std::shared_ptr<skity::Texture> WrapTexture(skity::GPUContext* context) {
    if (!Valid() || !context ||
        context->GetBackendType() != skity::GPUBackendType::kOpenGL) {
      return {};
    }
    skity::GPUBackendTextureInfoGL info{};
    info.backend = skity::GPUBackendType::kOpenGL;
    info.format = skity::TextureFormat::kRGBA;
    info.width = w_;
    info.height = h_;
    // VideoShaderYUV outputs straight RGB and alpha, not premultiplied RGB.
    info.alpha_type = skity::AlphaType::kUnpremul_AlphaType;
    info.tex_id = texture_;
    info.owned_by_engine = false;
    return context->WrapTexture(&info);
  }
};

std::unique_ptr<VideoShader> VideoShader::Make(
    std::shared_ptr<AnimaXAbility> ability) {
  return std::make_unique<VideoShaderLinux>(std::move(ability));
}

VideoShaderLinux::VideoShaderLinux(std::shared_ptr<AnimaXAbility> ability) {
  auto linux_ability = std::static_pointer_cast<AnimaXAbilityLinux>(ability);
  raster_runner_ = linux_ability->GetRasterRunner();
  // Tasks and recorded images retain the GL state independently of the layer.
  // The final release always runs on the context's raster thread.
  state_ = std::shared_ptr<State>(
      new State(), [runner = raster_runner_](State* state) {
        fml::TaskRunner::RunNowOrPostTask(runner, [state]() {
          const auto previous = ThreadAssert::GetCurrent();
          ThreadAssert::Init(ThreadAssert::Type::kGPU);
          delete state;
          ThreadAssert::Init(previous);
        });
      });
}

bool VideoShaderLinux::Valid() {
  return width_ > 0 && height_ > 0 && !state_->failed.load();
}

void VideoShaderLinux::Init(int32_t w, int32_t h,
                            const std::array<float, 4>& rgb_frame,
                            const std::array<float, 4>& a_frame) {
  width_ = w;
  height_ = h;
  if (w <= 0 || h <= 0) {
    state_->failed.store(true);
    return;
  }
  fml::TaskRunner::RunNowOrPostTask(
      raster_runner_, [state = state_, w, h, rgb_frame, a_frame]() {
        state->Init(w, h, rgb_frame, a_frame);
        const bool valid = state->Valid() && state->FramebufferComplete();
        state->failed.store(!valid);
      });
}

void VideoShaderLinux::Draw(std::unique_ptr<TextureInfo> texture_info,
                            const std::array<float, 16>& transform) {
  if (!Valid() || !texture_info) {
    return;
  }
  fml::TaskRunner::RunNowOrPostTask(
      raster_runner_,
      [state = state_, texture = std::move(texture_info), transform]() mutable {
        GLint active_texture, alignment, row_length;
        GLint bindings[3];
        GLboolean blend = glIsEnabled(GL_BLEND);
        GLboolean color_mask[4];
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
        glGetIntegerv(GL_UNPACK_ROW_LENGTH, &row_length);
        glGetBooleanv(GL_COLOR_WRITEMASK, color_mask);
        for (int i = 0; i < 3; ++i) {
          glActiveTexture(GL_TEXTURE0 + i);
          glGetIntegerv(GL_TEXTURE_BINDING_2D, &bindings[i]);
        }
        glActiveTexture(active_texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glDisable(GL_BLEND);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        state->Draw(std::move(texture), transform);
        glColorMask(color_mask[0], color_mask[1], color_mask[2], color_mask[3]);
        if (blend) glEnable(GL_BLEND);
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, row_length);
        for (int i = 0; i < 3; ++i) {
          glActiveTexture(GL_TEXTURE0 + i);
          glBindTexture(GL_TEXTURE_2D, bindings[i]);
        }
        glActiveTexture(active_texture);
      });
}

std::unique_ptr<Image> VideoShaderLinux::GetOutputImage(RealContext* context) {
  if (!Valid()) {
    return nullptr;
  }
  using Promise = SkityPromiseTexture<std::shared_ptr<State>>;
  auto image = Promise::MakeRetainedImage(
      state_,
      [](skity::GPUContext* gpu, std::shared_ptr<State> state) {
        return state->WrapTexture(gpu);
      },
      skity::TextureFormat::kRGBA, width_, height_,
      skity::AlphaType::kUnpremul_AlphaType);
  return std::make_unique<VideoImage>(std::move(image));
}

}  // namespace animax
}  // namespace lynx
