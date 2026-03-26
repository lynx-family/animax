// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/gl/scoped_gl_reset_restore.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

ScopedGLResetRestore::ScopedGLResetRestore(GLenum target) : target_(target) {
  switch (target_) {
    case GL_READ_FRAMEBUFFER_BINDING: {
      glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &int_[0]);
      break;
    }
    case GL_FRAMEBUFFER_BINDING: {
      glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &int_[0]);
      glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &int_[1]);
      break;
    }
    case GL_RENDERBUFFER_BINDING: {
      glGetIntegerv(GL_RENDERBUFFER_BINDING, &int_[0]);
      break;
    }
#if defined(OS_ANDROID) || defined(OS_HARMONY)
    case GL_TEXTURE_BINDING_EXTERNAL_OES: {
      glGetIntegerv(GL_TEXTURE_BINDING_EXTERNAL_OES, &int_[0]);
      break;
    }
#endif
    case GL_TEXTURE_BINDING_2D: {
      glGetIntegerv(GL_TEXTURE_BINDING_2D, &int_[0]);
      break;
    }
    case GL_CURRENT_PROGRAM: {
      glGetIntegerv(GL_CURRENT_PROGRAM, &int_[0]);
      break;
    }
    case GL_VERTEX_ARRAY_BINDING: {
      glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &int_[0]);
      break;
    }
    case GL_ARRAY_BUFFER_BINDING: {
      glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &int_[0]);
      break;
    }
    case GL_ELEMENT_ARRAY_BUFFER_BINDING: {
      glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &int_[0]);
      break;
    }
    case GL_ACTIVE_TEXTURE: {
      glGetIntegerv(GL_ACTIVE_TEXTURE, &int_[0]);
      break;
    }
    case GL_VIEWPORT: {
      glGetIntegerv(GL_VIEWPORT, &int_[0]);
      break;
    }
    case GL_COLOR_CLEAR_VALUE: {
      glGetFloatv(GL_COLOR_CLEAR_VALUE, &float_[0]);
      break;
    }
    case GL_COLOR_WRITEMASK: {
      glGetIntegerv(GL_COLOR_WRITEMASK, &int_[0]);
      break;
    }
    case GL_DEPTH_CLEAR_VALUE: {
      glGetFloatv(GL_DEPTH_CLEAR_VALUE, &float_[0]);
      break;
    }
    case GL_DEPTH_WRITEMASK: {
      glGetIntegerv(GL_DEPTH_WRITEMASK, &int_[0]);
      break;
    }
    case GL_STENCIL_CLEAR_VALUE: {
      glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &int_[0]);
      break;
    }
    case GL_STENCIL_WRITEMASK: {
      glGetIntegerv(GL_STENCIL_WRITEMASK, &int_[0]);
      break;
    }

    case GL_BLEND:
      [[fallthrough]];
    case GL_CULL_FACE:
      [[fallthrough]];
    case GL_SCISSOR_TEST:
      [[fallthrough]];
    case GL_STENCIL_TEST:
      [[fallthrough]];
    case GL_DEPTH_TEST: {
      glGetIntegerv(target, &int_[0]);
      break;
    }
    case GL_BLEND_SRC_RGB:
      [[fallthrough]];
    case GL_BLEND_SRC_ALPHA:
      [[fallthrough]];
    case GL_BLEND_DST_RGB:
      [[fallthrough]];
    case GL_BLEND_DST_ALPHA: {
      // restore all if any one set
      glGetIntegerv(GL_BLEND_SRC_RGB, &int_[0]);
      glGetIntegerv(GL_BLEND_DST_RGB, &int_[1]);
      glGetIntegerv(GL_BLEND_SRC_ALPHA, &int_[2]);
      glGetIntegerv(GL_BLEND_DST_ALPHA, &int_[3]);
      break;
    }
    default: {
      DCHECK(false);
      break;
    }
  }
}

ScopedGLResetRestore::~ScopedGLResetRestore() {
  switch (target_) {
    case GL_READ_FRAMEBUFFER_BINDING: {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, int_[0]);
      break;
    }
    case GL_FRAMEBUFFER_BINDING: {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, int_[0]);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, int_[1]);
      break;
    }
    case GL_RENDERBUFFER_BINDING: {
      glBindRenderbuffer(GL_RENDERBUFFER, int_[0]);
      break;
    }
#if defined(OS_ANDROID) || defined(OS_HARMONY)
    case GL_TEXTURE_BINDING_EXTERNAL_OES: {
      glBindTexture(GL_TEXTURE_EXTERNAL_OES, int_[0]);
      break;
    }
#endif
    case GL_TEXTURE_BINDING_2D: {
      glBindTexture(GL_TEXTURE_2D, int_[0]);
      break;
    }
    case GL_CURRENT_PROGRAM: {
      auto p = int_[0];
      if (p == GL_NONE) {
        glUseProgram(GL_NONE);
      } else {
        if (glIsProgram(p)) {
          GLint linked;
          glGetProgramiv(p, GL_LINK_STATUS, &linked);
          if (linked == GL_TRUE) {
            glUseProgram(p);
          } else {
            // maybe shader is detached
            glUseProgram(GL_NONE);
            ANIMAX_LOGW("ScopedGLState restore but program is not linked.");
          }
        } else {
          glUseProgram(GL_NONE);
          ANIMAX_LOGW("ScopedGLState restore but stored program is invalid.");
        }
      }
      break;
    }
    case GL_VERTEX_ARRAY_BINDING: {
      glBindVertexArray(int_[0]);
      break;
    }
    case GL_ARRAY_BUFFER_BINDING: {
      glBindBuffer(GL_ARRAY_BUFFER, int_[0]);
      break;
    }
    case GL_ELEMENT_ARRAY_BUFFER_BINDING: {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, int_[0]);
      break;
    }
    case GL_ACTIVE_TEXTURE: {
      glActiveTexture(int_[0]);
      break;
    }
    case GL_VIEWPORT: {
      glViewport(int_[0], int_[1], int_[2], int_[3]);
      break;
    }
    case GL_COLOR_CLEAR_VALUE: {
      glClearColor(float_[0], float_[1], float_[2],
                   float_[3]);  // This probably needs more detail
      break;
    }
    case GL_COLOR_WRITEMASK: {
      glColorMask(int_[0], int_[1], int_[2], int_[3]);
      break;
    }
    case GL_DEPTH_CLEAR_VALUE: {
      glClearDepthf(float_[0]);
      break;
    }
    case GL_DEPTH_WRITEMASK: {
      glDepthMask(int_[0]);
      break;
    }
    case GL_STENCIL_CLEAR_VALUE: {
      glClearStencil(int_[0]);
      break;
    }
    case GL_STENCIL_WRITEMASK: {
      glStencilMask(int_[0]);
      break;
    }

    case GL_BLEND:
      [[fallthrough]];
    case GL_CULL_FACE:
      [[fallthrough]];
    case GL_SCISSOR_TEST:
      [[fallthrough]];
    case GL_STENCIL_TEST:
      [[fallthrough]];
    case GL_DEPTH_TEST: {
      if (int_[0]) {
        glEnable(target_);
      } else {
        glDisable(target_);
      }
      break;
    }
    case GL_BLEND_SRC_RGB:
      [[fallthrough]];
    case GL_BLEND_SRC_ALPHA:
      [[fallthrough]];
    case GL_BLEND_DST_RGB:
      [[fallthrough]];
    case GL_BLEND_DST_ALPHA: {
      // restore all if any one set
      glBlendFuncSeparate(int_[0], int_[1], int_[2], int_[3]);
      break;
    }
    default: {
      DCHECK(false);
      break;
    }
  }
}

}  // namespace animax
}  // namespace lynx
