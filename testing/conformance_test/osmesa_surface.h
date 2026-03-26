// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_TESTING_CONFORMANCE_TEST_OSMESA_SURFACE_H_
#define ANIMAX_TESTING_CONFORMANCE_TEST_OSMESA_SURFACE_H_

#include "src/base/gl/gl_include.h"

namespace lynx {
namespace animax {

class GLSurfaceOSMesa {
 public:
  GLSurfaceOSMesa(int32_t width, int32_t height);

  void* Buffer() { return buffer_; }

  const void* Buffer() const { return buffer_; }

  void Flush();

  GLuint Framebuffer() const { return 0; }

  bool MakeCurrent();

 private:
  int32_t width_ = 1;
  int32_t height_ = 1;
  void* buffer_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_TESTING_CONFORMANCE_TEST_OSMESA_SURFACE_H_
