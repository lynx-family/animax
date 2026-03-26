// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_GL_HARMONY_EGL_UTIL_H_
#define ANIMAX_SRC_BASE_GL_HARMONY_EGL_UTIL_H_

#include <native_image/native_image.h>
#include <native_window/external_window.h>

#include <string>
#include <vector>

#include "src/base/gl/gl_include.h"

namespace lynx {
namespace animax {

EGLDisplay GetEGLDefaultDisplay();
const std::vector<std::string>& GetEGLExtensions();
bool IsEGLExtensionSupported(const std::string& extension);
bool IsSurfacelessContextSupported();
bool IsNoConfigContextSupported();
EGLConfig ChooseEGLConfig(EGLDisplay display, bool stencil = true);
EGLConfig GetEGLConfigRGBA8Stencil8();
EGLConfig GetEGLConfigRGBA8();
EGLSurface CreateEGLSurface(EGLDisplay display, EGLConfig config,
                            OHNativeWindow* window);
bool DestroyEGLSurface(EGLDisplay display, EGLSurface surface);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_GL_HARMONY_EGL_UTIL_H_
