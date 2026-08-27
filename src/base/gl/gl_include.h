// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_GL_GL_INCLUDE_H_
#define ANIMAX_SRC_BASE_GL_GL_INCLUDE_H_

// clang-format off
#if defined(OS_ANDROID)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>
#elif defined(OS_HARMONY)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>
#elif defined(OS_IOS)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif defined(ANIMAX_GL_USE_OSMESA)
#include <GL/osmesa.h>
#include "third_party/mesa_build_support/include/mesa_gl.h"
#elif defined(OS_LINUX)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3platform.h>
#elif defined(OS_WIN)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3platform.h>
#elif defined(OS_OSX)
#include <OpenGL/gl3.h>
#define glInvalidateFramebuffer(target, numAttachments, attachments) ((void)0)
#elif defined(OS_WASM)
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#endif
// clang-format on

#endif  // ANIMAX_SRC_BASE_GL_GL_INCLUDE_H_
