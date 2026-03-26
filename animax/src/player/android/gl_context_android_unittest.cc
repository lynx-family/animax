// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/gl_context_android.h"

#include "gtest/gtest.h"
#include "src/base/gl/android/egl_util.h"
#include "src/base/gl/gl_include.h"
#include "testing/gl_test_util_android.h"

struct AnimaXEGLContextTestParams {
  bool is_no_config_context_supported = false;
  bool is_surfaceless_context_supported = false;
};

class AnimaXEGLContextTest
    : public ::testing::TestWithParam<AnimaXEGLContextTestParams> {
 public:
  std::unique_ptr<lynx::animax::AnimaXEGLContext> context_;
  EGLSurface draw_surface_ = EGL_NO_SURFACE;

  void SetUp() override {
    auto params = GetParam();
    context_ = std::make_unique<lynx::animax::AnimaXEGLContext>(
        params.is_no_config_context_supported,
        params.is_surfaceless_context_supported);
    EXPECT_EQ(eglGetError(), EGL_SUCCESS);
    const EGLint attrib_list_surface[] = {EGL_WIDTH, 2, EGL_HEIGHT, 2,
                                          EGL_NONE};
    draw_surface_ = eglCreatePbufferSurface(
        lynx::animax::GetEGLDefaultDisplay(), lynx::animax::GetEGLConfigRGBA8(),
        attrib_list_surface);
    EXPECT_NE(draw_surface_, EGL_NO_SURFACE);
  }

  void TearDown() override {
    context_.reset();
    eglDestroySurface(lynx::animax::GetEGLDefaultDisplay(), draw_surface_);
    ASSERT_EQ(eglGetError(), EGL_SUCCESS);
    ASSERT_EQ(eglGetCurrentContext(), EGL_NO_CONTEXT);
    ASSERT_EQ(eglGetCurrentSurface(EGL_READ), EGL_NO_SURFACE);
    ASSERT_EQ(eglGetCurrentSurface(EGL_DRAW), EGL_NO_SURFACE);
  }
};

TEST_P(AnimaXEGLContextTest, ContextValid) {
  using namespace lynx::animax;
  const auto is_no_config_context_supported_param =
      GetParam().is_no_config_context_supported;
  const auto is_surfaceless_context_supported_param =
      GetParam().is_surfaceless_context_supported;
  const auto is_no_config_context_supported_device =
      IsNoConfigContextSupported();
  const auto is_surfaceless_context_supported_device =
      IsSurfacelessContextSupported();

  if ((is_no_config_context_supported_param &&
       !is_no_config_context_supported_device) ||
      (is_surfaceless_context_supported_param &&
       !is_surfaceless_context_supported_device)) {
    return;
  }

  ASSERT_TRUE(context_->Valid());
}

TEST_P(AnimaXEGLContextTest, MakeCurrentWork) {
  using namespace lynx::animax;
  const EGLint attrib_list_surface[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
  EGLSurface surface1 = eglCreatePbufferSurface(
      GetEGLDefaultDisplay(), GetEGLConfigRGBA8Stencil8(), attrib_list_surface);
  EGLSurface surface2 = eglCreatePbufferSurface(
      GetEGLDefaultDisplay(), GetEGLConfigRGBA8Stencil8(), attrib_list_surface);
  // Neither EGL_DRAW or EGL_READ is EGL_NO_SURFACE
  ASSERT_TRUE(context_->MakeCurrent(surface1, surface2));
  ASSERT_TRUE(context_->IsCurrent());
  ASSERT_EQ(eglGetError(), EGL_SUCCESS);
  ASSERT_EQ(eglGetCurrentSurface(EGL_DRAW), surface1);
  ASSERT_EQ(eglGetCurrentSurface(EGL_READ), surface2);

  // Either one of EGL_DRAW and EGL_READ is EGL_NO_SURFACE
  ASSERT_TRUE(context_->MakeCurrent(EGL_NO_SURFACE, EGL_NO_SURFACE));
  ASSERT_TRUE(context_->IsCurrent());
  ASSERT_EQ(eglGetError(), EGL_SUCCESS);
  if (GetParam().is_surfaceless_context_supported) {
    ASSERT_EQ(eglGetCurrentSurface(EGL_DRAW), EGL_NO_SURFACE);
    ASSERT_EQ(eglGetCurrentSurface(EGL_READ), EGL_NO_SURFACE);
  } else {
    // Should be the fake surface.
    ASSERT_NE(eglGetCurrentSurface(EGL_DRAW), EGL_NO_SURFACE);
    ASSERT_NE(eglGetCurrentSurface(EGL_READ), EGL_NO_SURFACE);
    ASSERT_NE(eglGetCurrentSurface(EGL_DRAW), surface1);
    ASSERT_NE(eglGetCurrentSurface(EGL_DRAW), surface2);
    ASSERT_NE(eglGetCurrentSurface(EGL_READ), surface1);
    ASSERT_NE(eglGetCurrentSurface(EGL_READ), surface2);
  }

  // Both EGL_DRAW and EGL_READ are EGL_NO_SURFACE
  ASSERT_TRUE(context_->MakeCurrent(EGL_NO_SURFACE, EGL_NO_SURFACE));
  ASSERT_TRUE(context_->IsCurrent());
  ASSERT_EQ(eglGetError(), EGL_SUCCESS);
  if (GetParam().is_surfaceless_context_supported) {
    ASSERT_EQ(eglGetCurrentSurface(EGL_DRAW), EGL_NO_SURFACE);
    ASSERT_EQ(eglGetCurrentSurface(EGL_READ), EGL_NO_SURFACE);
  } else {
    ASSERT_NE(eglGetCurrentSurface(EGL_DRAW), EGL_NO_SURFACE);
    ASSERT_NE(eglGetCurrentSurface(EGL_READ), EGL_NO_SURFACE);
  }
}

TEST_P(AnimaXEGLContextTest, ContextCanDraw) {
  using namespace lynx::animax;
  ASSERT_TRUE(context_->MakeCurrent(draw_surface_, draw_surface_));
  ASSERT_EQ(eglGetCurrentSurface(EGL_DRAW), draw_surface_);
  ASSERT_EQ(eglGetCurrentSurface(EGL_READ), draw_surface_);
  glViewport(0, 0, 2, 2);
  glClearColor(0, 1, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glFlush();
  auto buffer = lynx::animax::testing::ReadPixels(2, 2);
  const auto clear_color_pixel =
      lynx::animax::testing::RGBAPixel(0, 255, 0, 255);
  for (int i = 0; i < 2 * 2; ++i) {
    EXPECT_EQ(clear_color_pixel, buffer[i]);
  }
}

INSTANTIATE_TEST_SUITE_P(
    AnimaXEGLContextTests, AnimaXEGLContextTest,
    ::testing::Values(AnimaXEGLContextTestParams{false, false},
                      AnimaXEGLContextTestParams{true, false},
                      AnimaXEGLContextTestParams{false, true},
                      AnimaXEGLContextTestParams{true, true}));
