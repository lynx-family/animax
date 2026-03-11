// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_EGL_SURFACE_H_
#define ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_EGL_SURFACE_H_

#include <android/native_window.h>

#include "src/base/gl/gl_include.h"
#include "src/player/android/gl_context_android.h"

namespace lynx {
namespace animax {

class AnimaXEGLSurface {
 public:
  AnimaXEGLSurface() = default;

  AnimaXEGLSurface(ANativeWindow* window, EGLConfig config);

  AnimaXEGLSurface(const AnimaXEGLSurface&) = delete;
  AnimaXEGLSurface& operator=(const AnimaXEGLSurface&) = delete;

  AnimaXEGLSurface(AnimaXEGLSurface&&);
  AnimaXEGLSurface& operator=(AnimaXEGLSurface&&);

  ~AnimaXEGLSurface();

  bool Valid() const;
  operator bool() const;

  void Flush();

  bool MakeCurrent();

  GLuint Framebuffer() {
    // EGLSurface's framebuffer is always the default framebuffer.
    return 0;
  }

  void Destroy();

 private:
  void Invalidate();
  EGLDisplay display_ = EGL_NO_DISPLAY;
  EGLSurface surface_ = EGL_NO_SURFACE;
  AnimaXEGLContextHolder eglContextHolder_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_EGL_SURFACE_H_
