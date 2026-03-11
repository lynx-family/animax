// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/gl/harmony/egl_util.h"

#include "base/include/no_destructor.h"
#include "src/base/gl/gl_util.h"
#include "src/base/gl/harmony/gl_context_harmony.h"
#include "src/base/log/log.h"

namespace {
static constexpr const char* kEGLKHRNoConfigContext =
    "EGL_KHR_no_config_context";
static constexpr const char* kEGLKHRSurfacelessContext =
    "EGL_KHR_surfaceless_context";

}  // namespace

namespace lynx {
namespace animax {
EGLDisplay GetEGLDefaultDisplay() {
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display != EGL_NO_DISPLAY && eglInitialize(display, nullptr, nullptr)) {
    return display;
  } else {
    ANIMAX_LOGE("eglGetDisplay(EGL_DEFAULT_DISPLAY) failed");
    return EGL_NO_DISPLAY;
  }
}

std::vector<std::string> SplitAndSortEGLExtensions() {
  const auto* extensions = reinterpret_cast<const char*>(
      eglQueryString(GetEGLDefaultDisplay(), EGL_EXTENSIONS));
  if (!extensions) {
    return std::vector<std::string>{};
  }

  return SplitAndSortExtensions(extensions);
}

const std::vector<std::string>& GetEGLExtensions() {
  static base::NoDestructor<std::vector<std::string>> extensions =
      base::NoDestructor<std::vector<std::string>>{
          std::vector<std::string>{SplitAndSortEGLExtensions()}};
  return *extensions;
}

bool IsEGLExtensionSupported(const std::string& extension) {
  return IsExtensionSupported(GetEGLExtensions(), extension);
}

bool IsSurfacelessContextSupported() {
  return IsEGLExtensionSupported(kEGLKHRSurfacelessContext);
}

bool IsNoConfigContextSupported() {
  return IsEGLExtensionSupported(kEGLKHRNoConfigContext);
}

EGLConfig ChooseEGLConfig(EGLDisplay display, bool stencil) {
  EGLint attributes[] = {
      // clang-format off
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
            EGL_RED_SIZE,        8,
            EGL_GREEN_SIZE,      8,
            EGL_BLUE_SIZE,       8,
            EGL_ALPHA_SIZE,      8,
            EGL_DEPTH_SIZE,      0,
            EGL_STENCIL_SIZE,    stencil ? 8 : 0,
            EGL_NONE,            // termination sentinel
      // clang-format on
  };

  EGLint config_count = 0;
  EGLConfig config = nullptr;

  if (!eglChooseConfig(display, attributes, &config, 1, &config_count)) {
    return nullptr;
  }

  if (config == nullptr || config_count == 0) {
    return nullptr;
  }

  // eglChooseConfig return ture && config_count > 0 && config != nullptr
  return config;
}

EGLConfig GetEGLConfigRGBA8Stencil8() {
  return ChooseEGLConfig(GetEGLDefaultDisplay(), true);
}

EGLConfig GetEGLConfigRGBA8() {
  return ChooseEGLConfig(GetEGLDefaultDisplay(), false);
}

EGLSurface CreateEGLSurface(EGLDisplay display, EGLConfig config,
                            OHNativeWindow* window) {
  if (display == EGL_NO_DISPLAY || config == EGL_NO_CONFIG_KHR ||
      window == nullptr) {
    ANIMAX_LOGE(
        "Failed to create EGLSurface, "
        "display: "
        << display << " config: " << config << " window: " << window);
    return EGL_NO_SURFACE;
  }

  // width != 0 && height != 0 && display_ != EGL_NO_DISPLAY
  static const EGLint egl_create_window_surface_attrib_list = {EGL_NONE};
  auto surface = eglCreateWindowSurface(
      display, config, reinterpret_cast<EGLNativeWindowType>(window),
      &egl_create_window_surface_attrib_list);

  ANIMAX_LOGI("eglCreateWindowSurface, window: " << window
                                                 << ", surface: " << surface);

  if (surface == EGL_NO_SURFACE) {
    ANIMAX_LOGE("eglCreateWindowSurface fail, error: " << eglGetError());
  }

  return surface;
}

bool DestroyEGLSurface(EGLDisplay display, EGLSurface surface) {
  if (display == EGL_NO_DISPLAY || surface == EGL_NO_SURFACE) {
    return false;
  }
  // Make surface_ not current to any thread, then eglDestroySurface will
  // destroy it immediately.
  AnimaXEGLContext::Instance().MakeCurrent(EGL_NO_SURFACE, EGL_NO_SURFACE);
  if (eglDestroySurface(display, surface) != EGL_TRUE) {
    ANIMAX_LOGE("Failed to destroy egl surface.")
    return false;
  } else {
    return true;
  }
}
}  // namespace animax
}  // namespace lynx
