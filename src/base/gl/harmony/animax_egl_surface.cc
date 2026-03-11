// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/gl/harmony/animax_egl_surface.h"

#include "src/base/gl/harmony/egl_util.h"
#include "src/base/gl/harmony/gl_context_harmony.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

AnimaXEGLSurface::AnimaXEGLSurface(OHNativeWindow* window, EGLConfig config)
    : display_{GetEGLDefaultDisplay()},
      surface_{CreateEGLSurface(display_, config, window)} {}

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

AnimaXEGLSurface::~AnimaXEGLSurface() { Destroy(); }

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
  if (eglSwapBuffers(display_, surface_) != EGL_TRUE) {
    ANIMAX_LOGE("Failed to swap egl buffers.");
  }
}

bool AnimaXEGLSurface::Valid() const {
  return display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE;
}

AnimaXEGLSurface::operator bool() const { return Valid(); }

}  // namespace animax
}  // namespace lynx
