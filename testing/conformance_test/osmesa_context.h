// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_TESTING_CONFORMANCE_TEST_OSMESA_CONTEXT_H_
#define ANIMAX_TESTING_CONFORMANCE_TEST_OSMESA_CONTEXT_H_

#include <array>

#include "src/base/gl/gl_include.h"

namespace lynx {
namespace animax {
class AnimaXOSMesaContext {
 public:
  static AnimaXOSMesaContext& ThreadLocalInstance();
  ~AnimaXOSMesaContext();

  OSMesaContext Context() { return context_; }
  bool MakeCurrent(int width, int height, void* buffer);
  bool MakeCurrent();

 private:
  AnimaXOSMesaContext();
  std::array<uint32_t, 1> buffer_;
  OSMesaContext context_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_TESTING_CONFORMANCE_TEST_OSMESA_CONTEXT_H_
