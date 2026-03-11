// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_GL_GL_UTIL_H_
#define ANIMAX_SRC_BASE_GL_GL_UTIL_H_

#include <string>
#include <vector>

#include "include/base/macros.h"
#include "src/base/gl/gl_include.h"
#include "src/base/gl/scoped_gl_object.h"

namespace lynx {
namespace animax {

ANIMAX_EXPORT void *GetGLProcLoader();
void *LoadGLProc(const char *proc_name);

std::vector<std::string> SplitAndSortExtensions(const std::string &extensions);
bool IsExtensionSupported(const std::vector<std::string> &extensions,
                          const std::string &extension);

const std::vector<std::string> &GetGLExtensions();
bool IsGLExtensionSupported(const std::string &extension);

GLuint CreateProgram(const char *vs_code, const char *fs_code);
ScopedGLObject CreateFXAAProgram();
ScopedGLObject CreateTexImage2D(GLsizei width, GLsizei height);
ScopedGLObject CreateStencilBuffer(GLsizei width, GLsizei height);
ScopedGLObject CreateFramebuffer(const ScopedGLObject &color_attachment,
                                 GLuint stencil_buffer);
void AttachStencilBufferToFramebuffer(GLuint framebuffer,
                                      GLuint stencil_buffer);
void DiscardColorAttachment();
void DiscardStencilBuffer();

struct VAOVBOPair {
  ScopedGLObject vao_;
  ScopedGLObject vbo_;
};
VAOVBOPair CreateFullScreenQuadVAOVBOPair();

#if defined(OS_ANDROID)
bool IsTiledMSAASupported();
ScopedGLObject CreateMultisampleEXTStencilBuffer(GLsizei width, GLsizei height);
ScopedGLObject CreateMultisampleEXTFramebuffer(
    GLuint texture, GLuint multisample_stencil_buffer);
ScopedGLObject CreateTiledMSAAProgram();
#endif

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_GL_GL_UTIL_H_
