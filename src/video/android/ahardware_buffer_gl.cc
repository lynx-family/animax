// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/android/ahardware_buffer_gl.h"

#include "src/base/gl/gl_include.h"
#include "src/base/log/log.h"
#include "src/base/util/android/hardware_buffer_functions.h"

namespace lynx {
namespace animax {

namespace {

// EGL/GL extension function pointers (loaded via eglGetProcAddress).
using GetNativeClientBufferFn = EGLClientBuffer (*)(const AHardwareBuffer*);
using CreateImageFn = EGLImageKHR (*)(EGLDisplay, EGLContext, EGLenum,
                                      EGLClientBuffer, const EGLint*);
using DestroyImageFn = EGLBoolean (*)(EGLDisplay, EGLImageKHR);
using ImageTargetTexture2DFn = void (*)(GLenum, void*);

GetNativeClientBufferFn g_get_native_client_buffer = nullptr;
CreateImageFn g_create_image = nullptr;
DestroyImageFn g_destroy_image = nullptr;
ImageTargetTexture2DFn g_image_target_texture = nullptr;

bool LoadExtensions() {
  if (g_get_native_client_buffer != nullptr && g_create_image != nullptr &&
      g_destroy_image != nullptr && g_image_target_texture != nullptr) {
    return true;
  }
  g_get_native_client_buffer = reinterpret_cast<GetNativeClientBufferFn>(
      eglGetProcAddress("eglGetNativeClientBufferANDROID"));
  g_create_image =
      reinterpret_cast<CreateImageFn>(eglGetProcAddress("eglCreateImageKHR"));
  g_destroy_image =
      reinterpret_cast<DestroyImageFn>(eglGetProcAddress("eglDestroyImageKHR"));
  g_image_target_texture = reinterpret_cast<ImageTargetTexture2DFn>(
      eglGetProcAddress("glEGLImageTargetTexture2DOES"));

  if (g_get_native_client_buffer == nullptr || g_create_image == nullptr ||
      g_destroy_image == nullptr || g_image_target_texture == nullptr) {
    ANIMAX_LOGE("AHardwareBufferGL: failed to load EGL extensions: "
                << "getNativeClientBuffer=" << g_get_native_client_buffer
                << ", createImage=" << g_create_image
                << ", destroyImage=" << g_destroy_image
                << ", imageTargetTexture=" << g_image_target_texture);
    return false;
  }
  return true;
}

}  // namespace

AHardwareBufferGL::~AHardwareBufferGL() { Destroy(); }

bool AHardwareBufferGL::Init(int32_t width, int32_t height) {
  if (texture_ != 0) {
    return true;  // already initialized
  }
  if (!HardwareBufferFunctions::IsHardwareBufferSupported()) {
    ANIMAX_LOGE("AHardwareBufferGL: AHardwareBuffer not supported (SDK<26)");
    return false;
  }
  if (!LoadExtensions()) {
    return false;
  }

  // 1. Allocate AHardwareBuffer (RGBA8, GPU color output + sampled).
  AHardwareBuffer_Desc desc = {};
  desc.width = static_cast<uint32_t>(width);
  desc.height = static_cast<uint32_t>(height);
  desc.layers = 1;
  desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
  desc.usage = AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT |
               AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
  if (HardwareBufferFunctions::GetInstance().Allocate(&desc, &ahb_) != 0 ||
      ahb_ == nullptr) {
    ANIMAX_LOGE("AHardwareBufferGL: AHardwareBuffer_allocate failed");
    return false;
  }

  // 2. Create EGLImage from the AHardwareBuffer.
  // Use the display of the currently-bound EGL context, not the default
  // display: the EGLImage and all subsequent GL usage (image target texture,
  // FBO rendering) must live on the same display as the current context.
  // Cache it so Destroy() destroys the image on the exact display it was
  // created on, as required by eglDestroyImageKHR.
  display_ = eglGetCurrentDisplay();
  if (display_ == EGL_NO_DISPLAY) {
    ANIMAX_LOGE("AHardwareBufferGL: no current EGL context/display");
    Destroy();
    return false;
  }
  EGLClientBuffer client_buf = g_get_native_client_buffer(ahb_);
  if (client_buf == nullptr) {
    ANIMAX_LOGE("AHardwareBufferGL: eglGetNativeClientBufferANDROID failed");
    Destroy();
    return false;
  }
  EGLint image_attribs[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};
  egl_image_ =
      g_create_image(display_, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                     client_buf, image_attribs);
  if (egl_image_ == EGL_NO_IMAGE_KHR) {
    ANIMAX_LOGE(
        "AHardwareBufferGL: eglCreateImageKHR failed: " << eglGetError());
    Destroy();
    return false;
  }

  // 3. Bind EGLImage to a new GL texture (FBO color attachment target).
  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);
  g_image_target_texture(GL_TEXTURE_2D, egl_image_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);

  return true;
}

void AHardwareBufferGL::Destroy() {
  // GL texture objects are context-scoped and need a current context to delete.
  // Destroy() may run without one: the destructor is reached after
  // VideoShaderAndroid's EGL scope has released the context (or after a failed
  // MakeCurrent, when the unique_ptr still destructs the object). Guard the GL
  // delete so a context-less call is never issued; texture_ is still zeroed and
  // if it is skipped the driver reclaims it when its context is torn down. The
  // EGL image (destroyed via its display) and the native AHardwareBuffer (NDK
  // release) do not need a current context, so they are always reclaimed.
  bool has_current_context = eglGetCurrentContext() != EGL_NO_CONTEXT;
  if (texture_ != 0) {
    if (has_current_context) {
      glDeleteTextures(1, &texture_);
    }
    texture_ = 0;
  }
  if (egl_image_ != EGL_NO_IMAGE_KHR) {
    if (display_ != EGL_NO_DISPLAY && g_destroy_image != nullptr) {
      g_destroy_image(display_, egl_image_);
    }
    egl_image_ = EGL_NO_IMAGE_KHR;
  }
  if (ahb_ != nullptr) {
    HardwareBufferFunctions::GetInstance().Release(ahb_);
    ahb_ = nullptr;
  }
}

}  // namespace animax
}  // namespace lynx
