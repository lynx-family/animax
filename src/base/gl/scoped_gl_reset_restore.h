// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_GL_SCOPED_GL_RESET_RESTORE_H_
#define ANIMAX_SRC_BASE_GL_SCOPED_GL_RESET_RESTORE_H_

#include <array>
#include <initializer_list>
#include <vector>

#include "include/base/macros.h"
#include "src/base/gl/gl_include.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT ScopedGLResetRestore {
 public:
  explicit ScopedGLResetRestore(GLenum target);
  ~ScopedGLResetRestore();

 private:
  GLenum target_{};
  std::array<GLint, 4> int_{};
  std::array<GLfloat, 4> float_{};
};
class BundleScopedGLResetRestore {
 public:
  BundleScopedGLResetRestore(std::initializer_list<GLenum> flags) {
    restore_.reserve(flags.size());
    for (const auto flag : flags) {
      restore_.emplace_back(flag);
    }
  }

 private:
  std::vector<ScopedGLResetRestore> restore_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_GL_SCOPED_GL_RESET_RESTORE_H_
