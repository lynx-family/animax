// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/gl/scoped_gl_object.h"

#include <array>

#include "src/base/log/log.h"

using GLDeleteFuncPtr = decltype(&glDeleteFramebuffers);

#if defined(OS_WIN)
#define CONSTEXPR_OR_CONST const
#else
#define CONSTEXPR_OR_CONST constexpr
#endif

static CONSTEXPR_OR_CONST std::array<GLDeleteFuncPtr, 7> kDeleteFuncPtrs = {{
    nullptr,
    glDeleteFramebuffers,
    glDeleteRenderbuffers,
    glDeleteTextures,
    glDeleteVertexArrays,
    nullptr,
    glDeleteBuffers,
}};

namespace lynx {
namespace animax {

ScopedGLObject::ScopedGLObject(ScopedGLObject&& rhs) noexcept
    : type_{rhs.type_}, object_{rhs.object_} {
  rhs.type_ = kInvalid;
  rhs.object_ = 0;
}

ScopedGLObject& ScopedGLObject::operator=(ScopedGLObject&& rhs) noexcept {
  if (this == &rhs) {
    return *this;
  }

  Delete();
  std::swap(type_, rhs.type_);
  std::swap(object_, rhs.object_);
  return *this;
}

ScopedGLObject::~ScopedGLObject() { Delete(); }

void ScopedGLObject::Delete() {
  if (object_ <= 0) {
    return;
  }

  switch (type_) {
    case kProgram: {
      glDeleteProgram(object_);
      break;
    }
    case kFramebuffer:
      [[fallthrough]];
    case kRenderbuffer:
      [[fallthrough]];
    case kTexture:
      [[fallthrough]];
    case kVertexArray:
      [[fallthrough]];
    case kBuffer: {
      auto* deleteFuncPtr = kDeleteFuncPtrs[type_];
      if (deleteFuncPtr) {
        (*deleteFuncPtr)(1, &object_);
      }
      break;
    }
    case kInvalid:
      [[fallthrough]];
    default: {
      DCHECK(false);
      break;
    }
  }

  type_ = kInvalid;
  object_ = 0;
}

}  // namespace animax
}  // namespace lynx
