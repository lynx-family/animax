// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "testing/conformance_test/osmesa_surface.h"

#include <cassert>
#include <cstdlib>

#include "testing/conformance_test/osmesa_context.h"

namespace lynx {
namespace animax {

GLSurfaceOSMesa::GLSurfaceOSMesa(int32_t width, int32_t height)
    : width_(width), height_(height), buffer_(std::malloc(width * height * 4)) {
  assert(buffer_);
}

bool GLSurfaceOSMesa::MakeCurrent() {
  return AnimaXOSMesaContext::ThreadLocalInstance().MakeCurrent(width_, height_,
                                                                buffer_);
}

void GLSurfaceOSMesa::Flush() { glFlush(); }

}  // namespace animax
}  // namespace lynx
