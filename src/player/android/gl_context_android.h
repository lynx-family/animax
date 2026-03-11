// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_GL_CONTEXT_ANDROID_H_
#define ANIMAX_SRC_PLAYER_ANDROID_GL_CONTEXT_ANDROID_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <string>
#include <vector>

namespace lynx {
namespace animax {

class AnimaXEGLContextHolder {
 public:
  AnimaXEGLContextHolder();
  ~AnimaXEGLContextHolder();
  AnimaXEGLContextHolder(const AnimaXEGLContextHolder&) = delete;
  AnimaXEGLContextHolder& operator=(const AnimaXEGLContextHolder&) = delete;
};

class AnimaXEGLContext {
 public:
  static AnimaXEGLContext& Instance();
  void init(bool auto_destroy_context = false);
  bool Valid() const;
  EGLConfig Config() const;
  bool MakeCurrent(EGLSurface draw_surface = EGL_NO_SURFACE,
                   EGLSurface read_surface = EGL_NO_SURFACE);
  bool IsCurrent() const;
  AnimaXEGLContext() = default;
  AnimaXEGLContext(bool is_no_config_context_supported,
                   bool is_surfaceless_context_supported);
  ~AnimaXEGLContext();
  void Ref() { ref_count_++; }
  void UnRef() {
    ref_count_--;
    if (ref_count_ <= 0 && is_auto_destroy_context_) {
      DestroyContext();
    }
  }

 private:
  EGLConfig config_ = EGL_NO_CONFIG_KHR;
  EGLContext context_ = EGL_NO_CONTEXT;
  EGLSurface fake_surface_ = EGL_NO_SURFACE;
  EGLDisplay display_ = EGL_NO_DISPLAY;
  bool is_valid_ = false;
  int ref_count_{0};
  bool is_no_config_context_supported_ = false;
  bool is_surfaceless_context_supported_ = false;
  bool is_auto_destroy_context_ = false;
  bool is_initialized_ = false;

  void EnsureContext();
  void DestroyContext();

  EGLDisplay GetEGLDisplay();
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_GL_CONTEXT_ANDROID_H_
