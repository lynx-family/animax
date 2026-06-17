// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_ANDROID_AHARDWARE_BUFFER_GL_H_
#define ANIMAX_SRC_VIDEO_ANDROID_AHARDWARE_BUFFER_GL_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <android/hardware_buffer.h>

#include "include/base/macros.h"

namespace lynx {
namespace animax {

// Bridges an AHardwareBuffer to a GL texture so that GL rendering (the video
// FBO) lands into a buffer that Vulkan can later import as a texture.
//
// Used by the Vulkan video path: the existing GL video shader renders into the
// FBO whose color attachment is this texture; then the AHardwareBuffer is
// wrapped into a skity Vulkan texture via GPUContext::WrapTexture.
//
// The caller must have an EGL context current when calling Init() / Destroy().
class ANIMAX_EXPORT AHardwareBufferGL {
 public:
  AHardwareBufferGL() = default;
  ~AHardwareBufferGL();

  AHardwareBufferGL(const AHardwareBufferGL&) = delete;
  AHardwareBufferGL& operator=(const AHardwareBufferGL&) = delete;

  // Allocate the AHardwareBuffer and bind it to a new GL texture. Requires an
  // EGL context current. Returns false on failure (already initialized, SDK<26,
  // missing EGL extension, or allocation/import failure).
  bool Init(int32_t width, int32_t height);
  void Destroy();

  GLuint GetTexture() const { return texture_; }
  AHardwareBuffer* GetHardwareBuffer() const { return ahb_; }

 private:
  AHardwareBuffer* ahb_ = nullptr;
  EGLImageKHR egl_image_ = EGL_NO_IMAGE_KHR;
  EGLDisplay display_ = EGL_NO_DISPLAY;  // display the EGLImage was created on
  GLuint texture_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_GL_AHARDWARE_BUFFER_GL_H_
