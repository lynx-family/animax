// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/animax_egl_surface.h"

#include "src/base/gl/android/egl_util.h"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/player/android/gl_context_android.h"

namespace lynx {
namespace animax {

AnimaXEGLSurface::AnimaXEGLSurface(ANativeWindow* window, EGLConfig config)
    : display_{GetEGLDefaultDisplay()},
      surface_{CreateEGLSurface(display_, config, window)} {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
}

AnimaXEGLSurface::AnimaXEGLSurface(AnimaXEGLSurface&& other)
    : display_{other.display_}, surface_{other.surface_} {
  other.Invalidate();
}

AnimaXEGLSurface& AnimaXEGLSurface::operator=(AnimaXEGLSurface&& other) {
  Destroy();
  display_ = other.display_;
  surface_ = other.surface_;
  other.Invalidate();
  return *this;
}

AnimaXEGLSurface::~AnimaXEGLSurface() {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  Destroy();
}

void AnimaXEGLSurface::Destroy() {
  if (!Valid()) {
    return;
  }
  DestroyEGLSurface(display_, surface_);
  Invalidate();
}

void AnimaXEGLSurface::Invalidate() {
  surface_ = EGL_NO_SURFACE;
  display_ = EGL_NO_DISPLAY;
}

bool AnimaXEGLSurface::MakeCurrent() {
  if (!Valid()) {
    ANIMAX_LOGE("MakeCurrent invalid, this: " << this);
    return false;
  }
  return AnimaXEGLContext::Instance().MakeCurrent(surface_, surface_);
}

void AnimaXEGLSurface::Flush() {
  glFlush();  // Avoid the overflow of the command buffer caused by excessive
              // drawing commands on certain devices.
  if (eglSwapBuffers(display_, surface_) != EGL_TRUE) {
    EGLint error = eglGetError();
    ANIMAX_LOGE("Failed to swap egl buffers, error: "
                << error << ", this: " << this << ", surface: " << surface_);
  }
}

bool AnimaXEGLSurface::Valid() const {
  return display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE;
}

AnimaXEGLSurface::operator bool() const { return Valid(); }

}  // namespace animax
}  // namespace lynx
