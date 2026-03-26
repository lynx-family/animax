// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_TESTING_GL_TEST_UTIL_ANDROID_H_
#define ANIMAX_TESTING_GL_TEST_UTIL_ANDROID_H_

#include <vector>

namespace lynx {
namespace animax {
namespace testing {
struct RGBAPixel {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t a = 0;

  RGBAPixel() = default;

  RGBAPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
      : r(r), g(g), b(b), a(a) {}

  friend bool operator==(const RGBAPixel& lhs, const RGBAPixel& rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
  }
};

std::vector<RGBAPixel> ReadPixels(int width, int height);
void RenderTriangle();

}  // namespace testing
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_TESTING_GL_TEST_UTIL_ANDROID_H_
