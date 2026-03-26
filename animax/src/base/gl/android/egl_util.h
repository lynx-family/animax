// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_GL_ANDROID_EGL_UTIL_H_
#define ANIMAX_SRC_BASE_GL_ANDROID_EGL_UTIL_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <string>
#include <vector>

namespace lynx {
namespace animax {

EGLDisplay GetEGLDefaultDisplay();
const std::vector<std::string>& GetEGLExtensions();
bool IsEGLExtensionSupported(const std::string& extension);
bool IsSurfacelessContextSupported();
bool IsNoConfigContextSupported();
EGLConfig ChooseEGLConfig(EGLDisplay display, bool stencil = true);
EGLConfig GetEGLConfigRGBA8Stencil8();
EGLConfig GetEGLConfigRGBA8(EGLDisplay display = EGL_NO_DISPLAY);
EGLSurface CreateEGLSurface(EGLDisplay display, EGLConfig config,
                            ANativeWindow* window);
bool DestroyEGLSurface(EGLDisplay display, EGLSurface surface);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_GL_ANDROID_EGL_UTIL_H_
