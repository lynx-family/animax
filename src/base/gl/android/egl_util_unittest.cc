// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/gl/android/egl_util.h"

#include "gtest/gtest.h"

namespace {
bool CheckEGLExtension(const char* ext) {
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display == EGL_NO_DISPLAY) {
    return false;
  }
  const char* extensions = eglQueryString(display, EGL_EXTENSIONS);
  if (extensions == nullptr) {
    return false;
  }

  return std::strstr(extensions, ext) != nullptr;
}
}  // namespace

TEST(EGLUtilTest, GetEGLDefaultDisplayTest) {
  using namespace lynx::animax;
  EGLDisplay display = GetEGLDefaultDisplay();
  ASSERT_NE(display, EGL_NO_DISPLAY);
  EGLDisplay display2 = GetEGLDefaultDisplay();
  ASSERT_EQ(display, display2);
}

TEST(EGLUtilTest, GetEGLExtensionsTest) {
  using namespace lynx::animax;
  auto extensions = GetEGLExtensions();
  ASSERT_FALSE(extensions.empty());
}

TEST(EGLUtilTest, CheckEGLExtension) {
  using namespace lynx::animax;
  auto extensions = GetEGLExtensions();
  for (const auto& ext : extensions) {
    ASSERT_TRUE(CheckEGLExtension(ext.c_str()));
    ASSERT_TRUE(IsEGLExtensionSupported(ext));
  }
}

TEST(EGLUtilTest, IsSurfacelessContextSupportedTest) {
  using namespace lynx::animax;
  if (IsSurfacelessContextSupported()) {
    ASSERT_TRUE(CheckEGLExtension("EGL_KHR_surfaceless_context"));
  } else {
    ASSERT_FALSE(CheckEGLExtension("EGL_KHR_surfaceless_context"));
  }
}

TEST(EGLUtilTest, IsNoConfigContextSupported) {
  using namespace lynx::animax;
  if (IsNoConfigContextSupported()) {
    ASSERT_TRUE(CheckEGLExtension("EGL_KHR_no_config_context"));
  } else {
    ASSERT_FALSE(CheckEGLExtension("EGL_KHR_no_config_context"));
  }
}

TEST(ChooseEGLConfigTest, ValidateAttributesEGLConfigRGBA8) {
  using namespace lynx::animax;
  EGLDisplay display = GetEGLDefaultDisplay();
  ASSERT_NE(EGL_NO_DISPLAY, display);

  EGLConfig config = GetEGLConfigRGBA8();
  ASSERT_NE(nullptr, config);

  EGLint value;

  eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &value);
  EXPECT_TRUE(EGL_OPENGL_ES3_BIT & value);

  eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &value);
  EXPECT_TRUE(EGL_WINDOW_BIT & value);

  eglGetConfigAttrib(display, config, EGL_RED_SIZE, &value);
  EXPECT_EQ(8, value);

  eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &value);
  EXPECT_EQ(8, value);

  eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &value);
  EXPECT_EQ(8, value);

  eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &value);
  EXPECT_EQ(8, value);

  eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &value);
  EXPECT_EQ(0, value);

  eglGetConfigAttrib(display, config, EGL_STENCIL_SIZE, &value);
  EXPECT_EQ(0, value);
}

TEST(ChooseEGLConfigTest, ValidateAttributesEGLConfigRGBA8Stencil8) {
  using namespace lynx::animax;
  EGLDisplay display = GetEGLDefaultDisplay();
  ASSERT_NE(EGL_NO_DISPLAY, display);

  EGLConfig config = GetEGLConfigRGBA8Stencil8();
  ASSERT_NE(nullptr, config);

  EGLint value;

  eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &value);
  EXPECT_TRUE(EGL_OPENGL_ES3_BIT & value);

  eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &value);
  EXPECT_TRUE(EGL_WINDOW_BIT & value);

  eglGetConfigAttrib(display, config, EGL_RED_SIZE, &value);
  EXPECT_EQ(8, value);

  eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &value);
  EXPECT_EQ(8, value);

  eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &value);
  EXPECT_EQ(8, value);

  eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &value);
  EXPECT_EQ(8, value);

  eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &value);
  // On some devices, the returned EGL depth buffer might be 24 if stencil size
  // is 8.
  EXPECT_GE(value, 0);

  eglGetConfigAttrib(display, config, EGL_STENCIL_SIZE, &value);
  EXPECT_EQ(8, value);
}

TEST(EGLUtilTest, CreateEGLSurfaceOnInvalidArgument) {
  using namespace lynx::animax;
  EGLDisplay displays[] = {EGL_NO_DISPLAY,
                           reinterpret_cast<EGLDisplay>(0xffff)};
  EGLConfig configs[] = {EGL_NO_CONFIG_KHR,
                         reinterpret_cast<EGLConfig>(0xffff)};
  ANativeWindow* windows[] = {nullptr,
                              reinterpret_cast<ANativeWindow*>(0xffff)};

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 2; ++k) {
        auto* display = displays[i];
        auto* config = configs[j];
        auto* window = windows[k];
        if (display == EGL_NO_DISPLAY || config == EGL_NO_CONFIG_KHR ||
            window == nullptr) {
          // If one of the three parameters is invalid, then CreateEGLSurface
          // should return EGL_NO_SURFACE.
          EXPECT_EQ(EGL_NO_SURFACE, CreateEGLSurface(display, config, window));
        }
      }
    }
  }
}

TEST(EGLUtilTest, DestroyEGLSurfaceOnInvalidArgument) {
  using namespace lynx::animax;
  EGLDisplay displays[] = {EGL_NO_DISPLAY,
                           reinterpret_cast<EGLDisplay>(0xffff)};
  EGLSurface surfaces[] = {EGL_NO_SURFACE,
                           reinterpret_cast<EGLSurface>(0xffff)};

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      auto* display = displays[i];
      auto* surface = surfaces[j];
      if (display == EGL_NO_DISPLAY || surface == EGL_NO_SURFACE) {
        // If one of the two parameters is invalid, then DestroyEGLSurface
        // should be noop and return false.
        EXPECT_FALSE(DestroyEGLSurface(display, surface));
      }
    }
  }
}
