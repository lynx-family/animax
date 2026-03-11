// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_GL_HARMONY_GL_CONTEXT_HARMONY_H_
#define ANIMAX_SRC_BASE_GL_HARMONY_GL_CONTEXT_HARMONY_H_

#include <string>
#include <vector>

#include "src/base/gl/gl_include.h"

namespace lynx {
namespace animax {

class AnimaXEGLContext {
 public:
  static AnimaXEGLContext& Instance();
  bool Valid() const;
  EGLConfig Config() const;
  bool MakeCurrent(EGLSurface draw_surface = EGL_NO_SURFACE,
                   EGLSurface read_surface = EGL_NO_SURFACE);
  bool IsCurrent() const;
  AnimaXEGLContext() = default;
  AnimaXEGLContext(bool is_no_config_context_supported,
                   bool is_surfaceless_context_supported);
  ~AnimaXEGLContext();

 private:
  EGLConfig config_ = EGL_NO_CONFIG_KHR;
  EGLContext context_ = EGL_NO_CONTEXT;
  EGLSurface fake_surface_ = EGL_NO_SURFACE;
  bool is_valid_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_GL_HARMONY_GL_CONTEXT_HARMONY_H_
