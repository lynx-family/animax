// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/animax_egl_surface.h"

#include "gtest/gtest.h"
using namespace lynx::animax;

namespace {

void CheckAnimaXEGLSurfaceInvalid(AnimaXEGLSurface& surface) {
  EXPECT_FALSE(surface.Valid());
  EXPECT_FALSE(surface);
  EXPECT_EQ(0, surface.Framebuffer());
  // These operations should be noop.
  surface.MakeCurrent();
  surface.Flush();
  surface.Destroy();
  EXPECT_FALSE(surface.Valid());
  EXPECT_FALSE(surface);
}

}  // namespace

TEST(AnimaXEGLSurface, DefaultConstructor) {
  auto surface = AnimaXEGLSurface{};
  CheckAnimaXEGLSurfaceInvalid(surface);
}

TEST(AnimaXEGLSurface, MoveOperations) {
  auto surface1 = AnimaXEGLSurface{};
  auto surface2 = AnimaXEGLSurface{};
  surface1 = std::move(surface2);
  auto surface3 = std::move(surface1);
  CheckAnimaXEGLSurfaceInvalid(surface1);
  CheckAnimaXEGLSurfaceInvalid(surface2);
  CheckAnimaXEGLSurfaceInvalid(surface3);
}
