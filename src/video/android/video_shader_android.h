// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_ANDROID_VIDEO_SHADER_ANDROID_H_
#define ANIMAX_SRC_VIDEO_ANDROID_VIDEO_SHADER_ANDROID_H_

#include <memory>

#include "src/base/gl/gl_include.h"
#include "src/render/real_context.h"
#include "src/video/video_shader_gl.h"

namespace skity {
class GPUSemaphore;
}

namespace lynx {
namespace animax {
class AHardwareBufferGL;
class Canvas;
class Image;

class VideoShaderAndroid : public VideoShaderGL {
 public:
  explicit VideoShaderAndroid(ContextBackend backend);
  ~VideoShaderAndroid() override;

  const char *GetVertexShader() const override;
  const char *GetFragmentShader() const override;
  GLint GetTransformMatrixLocation() override;
  GLenum GetVideoTextureTarget() override;
  GLenum GetVideoTextureBindingPoint() override;

  // VideoShader overrides.
  // Vulkan backend: the returned scope makes the offscreen EGL context current
  // around GL work and, on destruction, bridges the GL output to Vulkan via
  // AHardwareBuffer + GPU semaphore.
  std::unique_ptr<FrameScope> BeginFrame(Canvas *canvas,
                                         RealContext *context) override;
  std::unique_ptr<Image> GetOutputImage(RealContext *context) override;

  // VideoShaderGL overrides.
  // Under Vulkan the shader ensures its own offscreen EGL context here so that
  // callers need not manage any GL scope around resource creation.
  void Init(int32_t w, int32_t h, const std::array<float, 4> &rgb_frame,
            const std::array<float, 4> &a_frame) override;

 private:
  // Vulkan frame scope: MakeCurrent on construction; EGL fence -> GPU
  // semaphore -> current surface and ReleaseCurrent on destruction.
  class VulkanFrameScope;

  // VideoShaderGL override: when bridging to Vulkan, return the
  // AHardwareBuffer-backed GL texture as the FBO color attachment.
  GLuint OnInitTexture() override;

  // Insert an EGL native fence after the GL draw and return its sync fd
  // (-1 on failure or already-signaled). GL context must be current.
  int CreateFenceFd();
  void EnsureGPUSemaphore(RealContext *context);

  ContextBackend backend_ = ContextBackend::kOpenGL;
  std::unique_ptr<AHardwareBufferGL> ahb_output_;
  // Reusable GPU semaphore; re-imported with a new fence fd every frame.
  std::shared_ptr<skity::GPUSemaphore> gpu_semaphore_;

  GLint transform_matrix_loc_ = -1;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_ANDROID_VIDEO_SHADER_ANDROID_H_
