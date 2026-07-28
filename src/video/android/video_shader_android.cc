// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/android/video_shader_android.h"

#include <unistd.h>

#include "skity/gpu/gpu_context.hpp"
#include "skity/gpu/gpu_context_vk.hpp"
#include "skity/gpu/gpu_semaphore.hpp"
#include "src/base/log/log.h"
#include "src/base/util/android/egl_ext_functions.h"
#include "src/player/android/gl_context_android.h"
#include "src/player/animax_ability.h"
#include "src/render/canvas.h"
#include "src/render/real_context.h"
#include "src/render/texture_info_gl.h"
#include "src/render/vk/image_vk.h"
#include "src/video/android/ahardware_buffer_gl.h"

namespace lynx {
namespace animax {

VideoShaderAndroid::VideoShaderAndroid(ContextBackend backend)
    : backend_(backend) {
  // Vulkan backend renders GL output into an AHardwareBuffer-backed texture so
  // the result can be imported as a Vulkan texture.
  if (backend_ == ContextBackend::kVulkan) {
    ahb_output_ = std::make_unique<AHardwareBufferGL>();
  }
}

VideoShaderAndroid::~VideoShaderAndroid() {
  // Vulkan path: GL resources live on an offscreen EGL context that the GPU
  // thread does not otherwise hold, so scope one around the deletes below. GL
  // path: ScopedEGLContext is a no-op (context already current).
  bool need_egl_scope =
      backend_ == ContextBackend::kVulkan &&
      (ahb_output_ != nullptr || texture_ != 0 || fbo_ != 0 || program_ != 0);
  ScopedEGLContext egl(need_egl_scope);
  bool gl_ready = egl.ready();

  // Release GL resources while the context is still current. For the Vulkan
  // path the base dtor runs after ReleaseCurrent(), so delete the base-owned
  // GL objects (fbo_/program_) here and zero them to avoid a second delete.
  // When MakeCurrent failed, skip the GL deletes but still zero the handles so
  // the base dtor does not issue GL calls without a current context.
  if (ahb_output_) {
    // texture_ (base) aliases the AHardwareBuffer-backed GL texture.
    if (gl_ready) {
      ahb_output_->Destroy();
    }
    // When gl_ready is false, skip the explicit Destroy() here. The unique_ptr
    // still runs ~AHardwareBufferGL -> Destroy() at member-destruction time,
    // whose eglGetCurrentContext() guard skips the context-less GL delete (the
    // GL texture leaks but is reclaimed when its context is torn down) and
    // still releases the EGL image and native AHardwareBuffer.
    texture_ = 0;
  } else if (texture_) {
    if (gl_ready) {
      glDeleteTextures(1, &texture_);
    }
    texture_ = 0;
  }
  if (fbo_) {
    if (gl_ready) {
      glDeleteFramebuffers(1, &fbo_);
    }
    fbo_ = 0;
  }
  if (program_) {
    if (gl_ready) {
      glDeleteProgram(program_);
    }
    program_ = 0;
  }
  // `egl` releases the offscreen EGL context here, before the base dtor runs.
}

GLuint VideoShaderAndroid::OnInitTexture() {
  if (ahb_output_ == nullptr) {
    return VideoShaderGL::OnInitTexture();  // GL backend: plain GL texture
  }
  if (!ahb_output_->Init(w_, h_)) {
    ANIMAX_LOGE("VideoShaderAndroid: AHardwareBufferGL init failed");
    return 0;
  }
  return ahb_output_->GetTexture();
}

// Scoped frame implementation for the Vulkan backend: makes the offscreen EGL
// context current on construction (so the caller's GL work runs against it) and
// commits the frame on destruction by bridging GL completion to Vulkan.
class VideoShaderAndroid::VulkanFrameScope : public VideoShader::FrameScope {
 public:
  VulkanFrameScope(VideoShaderAndroid &shader, Canvas *canvas,
                   RealContext *context)
      : shader_(shader), canvas_(canvas), context_(context) {
    ready_ = AnimaXEGLContext::Instance().MakeCurrent();
    if (!ready_) {
      ANIMAX_LOGE(
          "VideoShaderAndroid: VulkanFrameScope MakeCurrent failed, caller "
          "must skip GL work this frame");
    }
  }

  ~VulkanFrameScope() override {
    if (!ready_) {
      // Never made current: no GL work ran this frame, so there is nothing to
      // commit. Returning here also avoids dropping another caller's current
      // context, which MakeCurrent left untouched on failure.
      return;
    }
    // 1. Capture GL completion as a native fence fd (GL context still current).
    int fd = shader_.CreateFenceFd();
    // 2. Release the offscreen EGL context; GL processing is done.
    AnimaXEGLContext::Instance().ReleaseCurrent();
    // 3. Import the fence into a reusable GPU semaphore (Vulkan op, no GL).
    shader_.EnsureGPUSemaphore(context_);
    if (fd >= 0 && shader_.gpu_semaphore_ != nullptr && context_ != nullptr &&
        context_->Get() != nullptr) {
      skity::GPUSemaphoreImportInfoVK import_info = {};
      import_info.sync_fd = fd;  // ownership transferred to the driver
      context_->Get()->ImportSemaphore(shader_.gpu_semaphore_.get(),
                                       import_info);
      if (canvas_ != nullptr) {
        // Current frame's surface waits on this before GPU rendering begins.
        canvas_->AddExternalWaitSemaphore(shader_.gpu_semaphore_);
      }
    } else if (fd >= 0) {
      close(fd);
    }
  }

  bool Ready() const override { return ready_; }

 private:
  VideoShaderAndroid &shader_;
  Canvas *canvas_;
  RealContext *context_;
  bool ready_ = false;
};

void VideoShaderAndroid::Init(int32_t w, int32_t h,
                              const std::array<float, 4> &rgb_frame,
                              const std::array<float, 4> &a_frame) {
  // Under Vulkan the GPU thread has no GL context; scope one around the
  // resource creation below. No-op on the GL backend (context already current).
  ScopedEGLContext egl(backend_ == ContextBackend::kVulkan);
  if (egl.ready()) {
    VideoShaderGL::Init(w, h, rgb_frame, a_frame);
  } else {
    ANIMAX_LOGE(
        "VideoShaderAndroid::Init: EGL MakeCurrent failed, shader "
        "resources will not be created");
  }
}

std::unique_ptr<VideoShader::FrameScope> VideoShaderAndroid::BeginFrame(
    Canvas *canvas, RealContext *context) {
  if (backend_ != ContextBackend::kVulkan) {
    // GL backend: the context is already current; nothing to scope.
    return VideoShaderGL::BeginFrame(canvas, context);
  }
  return std::make_unique<VulkanFrameScope>(*this, canvas, context);
}

std::unique_ptr<Image> VideoShaderAndroid::GetOutputImage(
    RealContext *context) {
  if (backend_ != ContextBackend::kVulkan || ahb_output_ == nullptr) {
    return VideoShaderGL::GetOutputImage(context);
  }
  AHardwareBuffer *ahb = ahb_output_->GetHardwareBuffer();
  if (ahb == nullptr || context == nullptr || context->Get() == nullptr) {
    return nullptr;
  }

  skity::GPUBackendTextureExtInfoAHB ahb_ext = {};
  ahb_ext.type = skity::GPUBackendTextureExtType::kAndroidHardwareBuffer;
  ahb_ext.hardware_buffer = ahb;

  skity::GPUBackendTextureInfoVK info = {};
  info.backend = skity::GPUBackendType::kVulkan;
  info.format = skity::TextureFormat::kRGBA;
  info.alpha_type = skity::AlphaType::kPremul_AlphaType;
  info.width = static_cast<uint32_t>(w_);
  info.height = static_cast<uint32_t>(h_);
  info.ext = &ahb_ext;

  auto texture = context->Get()->WrapTexture(&info);
  if (texture == nullptr) {
    ANIMAX_LOGE("VideoShaderAndroid: WrapTexture returned null for AHB");
    return nullptr;
  }
  return std::make_unique<ImageVK>(std::move(texture));
}

int VideoShaderAndroid::CreateFenceFd() {
  if (!EglExtFunctions::IsFenceSyncSupported()) {
    ANIMAX_LOGW("VideoShaderAndroid: EGL native fence sync unavailable");
    return -1;
  }
  EglExtFunctions &fns = EglExtFunctions::GetInstance();
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  // Create a fresh native fence at the current point of the GL command stream.
  // EGL_NO_NATIVE_FENCE_FD_ANDROID explicitly requests a new (unsignaled) fence
  // instead of importing an existing fd; this is the documented value for
  // attrib_list and is equivalent to passing NULL, spelled out for clarity.
  const EGLint attrib_list[] = {EGL_SYNC_NATIVE_FENCE_FD_ANDROID,
                                EGL_NO_NATIVE_FENCE_FD_ANDROID, EGL_NONE};
  EGLSyncKHR sync =
      fns.CreateSync(display, EGL_SYNC_NATIVE_FENCE_ANDROID, attrib_list);
  if (sync == EGL_NO_SYNC_KHR) {
    return -1;
  }
  glFlush();  // ensure the fence is submitted to the GPU
  int fd = fns.DupNativeFenceFD(display, sync);
  fns.DestroySync(display, sync);
  // A valid fd (>= 0) carries the GPU completion signal; -1 means duplication
  // failed, NOT that the fence has already signaled.
  return fd;
}

void VideoShaderAndroid::EnsureGPUSemaphore(RealContext *context) {
  if (context == nullptr || context->Get() == nullptr) {
    return;
  }
  gpu_semaphore_ = context->Get()->CreateSemaphore();
  if (gpu_semaphore_ == nullptr) {
    ANIMAX_LOGW("VideoShaderAndroid: CreateSemaphore failed");
  }
}

const char *VideoShaderAndroid::GetVertexShader() const {
  static const char *vs = R"(#version 300 es
precision highp float;
precision mediump int;

out vec2 tc;

void main() {
    const vec2[] pos = vec2[4](
        vec2(-1.0, -1.0),
        vec2(1.0, -1.0),
        vec2(1.0, 1.0),
        vec2(-1.0, 1.0)
    );
    tc = vec2(0.5, 0.5) * (pos[gl_VertexID] + vec2(1.0, 1.0));
    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
}

)";
  return vs;
}

const char *VideoShaderAndroid::GetFragmentShader() const {
  static const char *fs = R"(#version 300 es
#extension GL_OES_EGL_image_external: require
#extension GL_OES_EGL_image_external_essl3 : enable
precision highp float;
precision mediump int;

in vec2 tc;
uniform samplerExternalOES video;
uniform highp mat4 transformMatrix;
uniform vec4 rgbFrame;
uniform vec4 aFrame;

out vec4 color;
void main() {
    vec2 alphaFrameCoord = vec2(tc.x * aFrame.z + aFrame.x, tc.y * aFrame.w + aFrame.y);
    alphaFrameCoord = (transformMatrix * vec4(alphaFrameCoord.x, 1.0 - alphaFrameCoord.y, 0.0, 1.0)).xy;
    float a = texture(video, alphaFrameCoord).r;
    vec2 rgbFrameCoord = vec2(tc.x * rgbFrame.z + rgbFrame.x, tc.y * rgbFrame.w + rgbFrame.y);
    rgbFrameCoord = (transformMatrix * vec4(rgbFrameCoord.x, 1.0 - rgbFrameCoord.y, 0.0, 1.0)).xy;
    color = vec4(texture(video, rgbFrameCoord).rgb, a);
}
)";
  return fs;
}

GLint VideoShaderAndroid::GetTransformMatrixLocation() {
  if (-1 == transform_matrix_loc_) {
    transform_matrix_loc_ = glGetUniformLocation(program_, "transformMatrix");
  }
  return transform_matrix_loc_;
}

GLenum VideoShaderAndroid::GetVideoTextureTarget() {
  return GL_TEXTURE_EXTERNAL_OES;
}

GLenum VideoShaderAndroid::GetVideoTextureBindingPoint() {
  return GL_TEXTURE_BINDING_EXTERNAL_OES;
}

std::unique_ptr<VideoShader> VideoShader::Make(
    std::shared_ptr<AnimaXAbility> ability) {
  ContextBackend backend =
      ability ? ability->GetBackend() : ContextBackend::kOpenGL;
  return std::unique_ptr<VideoShaderAndroid>(new VideoShaderAndroid(backend));
}

}  // namespace animax
}  // namespace lynx
