// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/gl_context_android.h"

#include "base/include/no_destructor.h"
#include "src/base/gl/android/egl_util.h"
#include "src/base/gl/gl_util.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

AnimaXEGLContextHolder::AnimaXEGLContextHolder() {
  AnimaXEGLContext::Instance().Ref();
}

AnimaXEGLContextHolder::~AnimaXEGLContextHolder() {
  AnimaXEGLContext::Instance().UnRef();
}

AnimaXEGLContext::AnimaXEGLContext(bool is_no_config_context_supported,
                                   bool is_surfaceless_context_supported)
    : is_no_config_context_supported_(is_no_config_context_supported),
      is_surfaceless_context_supported_(is_surfaceless_context_supported) {
  ANIMAX_LOGI("AnimaXEGLContext construct, is_no_config_context_supported: "
              << is_no_config_context_supported
              << ", is_surfaceless_context_supported: "
              << is_surfaceless_context_supported);
  EnsureContext();
}

void AnimaXEGLContext::init(bool auto_destroy_context) {
  if (is_initialized_) {
    // Allow downgrading auto-destroy (true -> false): the Vulkan video bridge
    // relies on the offscreen EGL context surviving across GL texture deletes,
    // so a later init(false) from a destructor must be able to relax an earlier
    // init(true). Never upgrade false -> true: a caller that created the
    // context without auto-destroy does not expect it to be torn down.
    if (is_auto_destroy_context_ && !auto_destroy_context) {
      is_auto_destroy_context_ = false;
    }
    ANIMAX_LOGI("AnimaXEGLContext initialized, is_auto_destroy_context_: "
                << is_auto_destroy_context_
                << ", auto_destroy_context: " << auto_destroy_context);
    return;
  }
  is_initialized_ = true;
  is_auto_destroy_context_ = auto_destroy_context;
  ANIMAX_LOGI("AnimaXEGLContext init, is_auto_destroy_context_: "
              << is_auto_destroy_context_
              << ", auto_destroy_context: " << auto_destroy_context);
}

AnimaXEGLContext::~AnimaXEGLContext() {
  ANIMAX_LOGI("AnimaXEGLContext destruct");
}

void AnimaXEGLContext::EnsureContext() {
  if (is_valid_) {
    // Now the context is already created, do nothing.
    return;
  }
  if (is_no_config_context_supported_) {
    // This device support "EGL_KHR_no_config_context" extension.
    // initialize EGLContext with "EGL_NO_CONFIG_KHR" as config.
    config_ = EGL_NO_CONFIG_KHR;
  } else {
    // This device doesn't support "EGL_KHR_no_config_context" extension.
    // initialize EGLContext with predefined config.
    config_ = GetEGLConfigRGBA8(GetEGLDisplay());
    ANIMAX_LOGI(
        "This device doesn't support \"EGL_KHR_no_config_context\" extension");
  }

  EGLint attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  context_ =
      eglCreateContext(GetEGLDisplay(), config_, EGL_NO_CONTEXT, attrib_list);
  if (context_ == EGL_NO_CONTEXT) {
    ANIMAX_LOGE("eglCreateContext failed, context_: " << context_);
    return;
  } else {
    ANIMAX_LOGI("eglCreateContext success, context_: " << context_);
  }

  if (!is_surfaceless_context_supported_) {
    // This device does not support "EGL_KHR_surfaceless_context" extension.
    // The EGLContext could not be made current with draw and read surface as
    // EGL_NO_SURFACE. We have to create a fake surface to make the context
    // current
    const EGLint attrib_list_surface[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1,
                                          EGL_NONE};
    // It could be possible that the device support "EGL_KHR_no_config_context"
    // but it does not support "EGL_KHR_surfaceless_context", we have to set the
    // predefined config_
    fake_surface_ = eglCreatePbufferSurface(GetEGLDisplay(),
                                            GetEGLConfigRGBA8(GetEGLDisplay()),
                                            attrib_list_surface);
    if (fake_surface_ == EGL_NO_SURFACE) {
      ANIMAX_LOGE("Failed to create a fake surface for EGLContext");
      return;
    }
  }

  is_valid_ = true;
}

void AnimaXEGLContext::DestroyContext() {
  ANIMAX_LOGE("AnimaXEGLContext DestroyContext");

  is_valid_ = false;

  EGLDisplay default_display = GetEGLDisplay();
  if (default_display == EGL_NO_DISPLAY) {
    return;
  }
  if (!eglMakeCurrent(default_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                      EGL_NO_CONTEXT)) {
    ANIMAX_LOGE("Failed to MakeCurrent");
  }
  if (context_ != EGL_NO_CONTEXT) {
    if (!eglDestroyContext(default_display, context_)) {
      ANIMAX_LOGE("Failed to destroy EGLContext: " << context_);
    } else {
      ANIMAX_LOGI("success to destroy EGLContext: " << context_);
    }
  }
  if (fake_surface_ != EGL_NO_SURFACE) {
    if (!eglDestroySurface(default_display, fake_surface_)) {
      ANIMAX_LOGE("Failed to destroy EGLSurface");
    }
  }

  config_ = EGL_NO_CONFIG_KHR;
  context_ = EGL_NO_CONTEXT;
  fake_surface_ = EGL_NO_SURFACE;
  display_ = EGL_NO_DISPLAY;
}

bool AnimaXEGLContext::MakeCurrent(EGLSurface draw_surface,
                                   EGLSurface read_surface) {
  if (!is_valid_) {
    ANIMAX_LOGE(
        "Try to MakeCurrent with an invalid EGLContext, so try to ensure "
        "first.");
    EnsureContext();
  }

  if (!is_valid_) {
    ANIMAX_LOGE("Try to MakeCurrent with an invalid EGLContext.");
    return false;
  }

  // TODO(linyiyi): cache current context and surface in thread_local variables
  // to reduce eglMakeCurrent calls
  bool success = false;
  if (draw_surface == EGL_NO_SURFACE || read_surface == EGL_NO_SURFACE) {
    // Make eglContext current without draw and read surface.
    if (fake_surface_ == EGL_NO_SURFACE) {
      // This device support "EGL_KHR_surfaceless_context" extension.
      success = eglMakeCurrent(GetEGLDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE,
                               context_);
      if (!success) {
        ANIMAX_LOGE("eglMakeCurrent Failed, error: "
                    << eglGetError() << ", draw_surface: " << EGL_NO_SURFACE
                    << ", read_surface: " << EGL_NO_SURFACE
                    << ", context: " << context_);
      }
    } else {
      // This device doesn't support "EGL_KHR_surfaceless_context" extension.
      // The EGLContext could be made current with draw and read surface as
      // fake_surface_.
      success = eglMakeCurrent(GetEGLDisplay(), fake_surface_, fake_surface_,
                               context_);
      if (!success) {
        ANIMAX_LOGE("eglMakeCurrent Failed, error: "
                    << eglGetError() << ", draw_surface: " << fake_surface_
                    << ", read_surface: " << fake_surface_
                    << ", context: " << context_);
      }
    }
  } else {
    success =
        eglMakeCurrent(GetEGLDisplay(), draw_surface, read_surface, context_);
    if (!success) {
      ANIMAX_LOGE("eglMakeCurrent Failed, error: "
                  << eglGetError() << ", draw_surface: " << draw_surface
                  << ", read_surface: " << read_surface
                  << ", context: " << context_);
    }
  }

  return success;
}

bool AnimaXEGLContext::IsCurrent() const {
  EGLContext current_context = eglGetCurrentContext();
  return current_context == context_;
}

void AnimaXEGLContext::ReleaseCurrent() {
  if (!is_valid_) {
    return;
  }
  EGLDisplay display = GetEGLDisplay();
  if (display == EGL_NO_DISPLAY) {
    return;
  }
  if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                      EGL_NO_CONTEXT)) {
    ANIMAX_LOGE("Failed to ReleaseCurrent, error: " << eglGetError());
  }
}

bool AnimaXEGLContext::Valid() const { return is_valid_; }

EGLConfig AnimaXEGLContext::Config() const { return config_; }

AnimaXEGLContext& AnimaXEGLContext::Instance() {
  static thread_local base::NoDestructor<AnimaXEGLContext> context{
      IsNoConfigContextSupported(), IsSurfacelessContextSupported()};
  return *context;
}

EGLDisplay AnimaXEGLContext::GetEGLDisplay() {
  if (display_ == EGL_NO_DISPLAY) {
    display_ = GetEGLDefaultDisplay();
    ANIMAX_LOGE("GetEGLDisplay, display: " << display_);
  }

  return display_;
}

ScopedEGLContext::ScopedEGLContext(bool ensure) : ensure_(ensure) {
  if (!ensure_) {
    return;
  }
  AnimaXEGLContext::Instance().init(/*auto_destroy_context=*/false);
  ready_ = AnimaXEGLContext::Instance().MakeCurrent();
  if (!ready_) {
    ANIMAX_LOGE(
        "ScopedEGLContext: MakeCurrent failed, GL work will be skipped");
  }
}

ScopedEGLContext::~ScopedEGLContext() {
  // Only release what we successfully made current; releasing without a prior
  // MakeCurrent would disturb a context owned by another scope/caller.
  if (ensure_ && ready_) {
    AnimaXEGLContext::Instance().ReleaseCurrent();
  }
}

}  // namespace animax
}  // namespace lynx
