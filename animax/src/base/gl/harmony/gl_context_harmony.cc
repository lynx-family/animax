// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/gl/harmony/gl_context_harmony.h"

#include "base/include/no_destructor.h"
#include "src/base/gl/gl_util.h"
#include "src/base/gl/harmony/egl_util.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

AnimaXEGLContext::AnimaXEGLContext(bool is_no_config_context_supported,
                                   bool is_surfaceless_context_supported) {
  if (is_no_config_context_supported) {
    // This device support "EGL_KHR_no_config_context" extension.
    // initialize EGLContext with "EGL_NO_CONFIG_KHR" as config.
    config_ = EGL_NO_CONFIG_KHR;
  } else {
    // This device doesn't support "EGL_KHR_no_config_context" extension.
    // initialize EGLContext with predefined config.
    config_ = GetEGLConfigRGBA8();
  }

  EGLint attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  context_ = eglCreateContext(GetEGLDefaultDisplay(), config_, EGL_NO_CONTEXT,
                              attrib_list);
  if (context_ == EGL_NO_CONTEXT) {
    ANIMAX_LOGE("eglCreateContext with EGL_NO_CONFIG_KHR failed");
    return;
  }

  if (!is_surfaceless_context_supported) {
    // This device does not support "EGL_KHR_surfaceless_context" extension.
    // The EGLContext could not be made current with draw and read surface as
    // EGL_NO_SURFACE. We have to create a fake surface to make the context
    // current
    const EGLint attrib_list_surface[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1,
                                          EGL_NONE};
    // It could be possible that the device support "EGL_KHR_no_config_context"
    // but it does not support "EGL_KHR_surfaceless_context", we have to set the
    // predefined config_
    fake_surface_ = eglCreatePbufferSurface(
        GetEGLDefaultDisplay(), GetEGLConfigRGBA8(), attrib_list_surface);
    if (fake_surface_ == EGL_NO_SURFACE) {
      ANIMAX_LOGE("Failed to create a fake surface for EGLContext");
      return;
    }
  }

  is_valid_ = true;
}

AnimaXEGLContext::~AnimaXEGLContext() {
  EGLDisplay default_display = GetEGLDefaultDisplay();
  if (default_display == EGL_NO_DISPLAY) {
    return;
  }
  eglMakeCurrent(default_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                 EGL_NO_CONTEXT);
  if (context_ != EGL_NO_CONTEXT) {
    eglDestroyContext(default_display, context_);
  }
  if (fake_surface_ != EGL_NO_SURFACE) {
    eglDestroySurface(default_display, fake_surface_);
  }
}

bool AnimaXEGLContext::MakeCurrent(EGLSurface draw_surface,
                                   EGLSurface read_surface) {
  if (!is_valid_) {
    ANIMAX_LOGE("Try to MakeCurrent with an invalid EGLContext.");
    return false;
  }
  bool success = false;
  if (draw_surface == EGL_NO_SURFACE || read_surface == EGL_NO_SURFACE) {
    // Make eglContext current without draw and read surface.
    if (fake_surface_ == EGL_NO_SURFACE) {
      // This device support "EGL_KHR_surfaceless_context" extension.
      success = eglMakeCurrent(GetEGLDefaultDisplay(), EGL_NO_SURFACE,
                               EGL_NO_SURFACE, context_);
    } else {
      // This device doesn't support "EGL_KHR_surfaceless_context" extension.
      // The EGLContext could be made current with draw and read surface as
      // fake_surface_.
      success = eglMakeCurrent(GetEGLDefaultDisplay(), fake_surface_,
                               fake_surface_, context_);
    }
  } else {
    success = eglMakeCurrent(GetEGLDefaultDisplay(), draw_surface, read_surface,
                             context_);
  }

  if (!success) {
    ANIMAX_LOGE("eglMakeCurrent Failed");
  }

  return success;
}

bool AnimaXEGLContext::IsCurrent() const {
  EGLContext current_context = eglGetCurrentContext();
  return current_context == context_;
}

bool AnimaXEGLContext::Valid() const { return is_valid_; }

EGLConfig AnimaXEGLContext::Config() const { return config_; }

AnimaXEGLContext& AnimaXEGLContext::Instance() {
  static thread_local base::NoDestructor<AnimaXEGLContext> context{
      IsNoConfigContextSupported(), IsSurfacelessContextSupported()};
  return *context;
}

}  // namespace animax
}  // namespace lynx
