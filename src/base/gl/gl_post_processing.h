// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_GL_GL_POST_PROCESSING_H_
#define ANIMAX_SRC_BASE_GL_GL_POST_PROCESSING_H_

#include <memory>

#include "src/base/gl/gl_include.h"

namespace lynx {
namespace animax {

class AnimaXGLPostProcessing {
 public:
  enum Type {
    kNoPostProcessing,
    kAntiAliasingFXAA,
#if defined(OS_ANDROID)
    kAntiAliasingTiledMSAA,
#endif
  };

  static bool IsSupported(enum Type type);

  static std::unique_ptr<AnimaXGLPostProcessing> MakeNoPostProcessing(
      GLuint native_surface_framebuffer);
  static std::unique_ptr<AnimaXGLPostProcessing> MakeAntiAliasing(
      enum Type type, GLsizei width, GLsizei height);

  virtual ~AnimaXGLPostProcessing() = default;
  virtual void ResolveToFramebuffer(GLuint target_framebuffer) = 0;
  virtual GLuint Framebuffer() = 0;
  virtual bool Valid() const = 0;

  enum Type Type() const { return type_; }

 protected:
  explicit AnimaXGLPostProcessing(enum Type type) : type_(type) {}

 private:
  enum Type type_ = kNoPostProcessing;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_GL_GL_POST_PROCESSING_H_
