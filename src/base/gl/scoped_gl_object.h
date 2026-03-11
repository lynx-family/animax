// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_GL_SCOPED_GL_OBJECT_H_
#define ANIMAX_SRC_BASE_GL_SCOPED_GL_OBJECT_H_

#include "src/base/gl/gl_include.h"

namespace lynx {
namespace animax {
class ScopedGLObject {
 public:
  enum Type {
    kInvalid = 0,
    kFramebuffer,
    kRenderbuffer,
    kTexture,
    kVertexArray,
    kProgram,
    kBuffer,
  };

  ScopedGLObject() = default;
  ScopedGLObject(Type type, GLuint object) : type_(type), object_(object) {}
  ScopedGLObject(const ScopedGLObject& rhs) = delete;
  ScopedGLObject& operator=(const ScopedGLObject& rhs) = delete;
  ScopedGLObject(ScopedGLObject&& rhs) noexcept;
  ScopedGLObject& operator=(ScopedGLObject&& rhs) noexcept;
  ~ScopedGLObject();

  operator decltype(auto)() { return object_; }

  operator decltype(auto)() const { return object_; }

  enum Type Type() const { return type_; }

 private:
  void Delete();
  enum Type type_ = kInvalid;
  GLuint object_ = 0;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_GL_SCOPED_GL_OBJECT_H_
