// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/web/video_shader_web.h"

#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/render/texture_info_gl.h"

namespace lynx {
namespace animax {

VideoShaderWeb::~VideoShaderWeb() {}

std::unique_ptr<VideoShader> VideoShader::Make() {
  return std::unique_ptr<VideoShaderWeb>(new VideoShaderWeb());
}

const char *VideoShaderWeb::GetVertexShader() const {
  // todo: impl
  return nullptr;
}

const char *VideoShaderWeb::GetFragmentShader() const {
  // todo: impl
  return nullptr;
}

GLint VideoShaderWeb::GetTransformMatrixLocation() {
  // todo: impl
  return -1;
}

GLenum VideoShaderWeb::GetVideoTextureTarget() {
  // todo: impl
  return -1;
}

GLenum VideoShaderWeb::GetVideoTextureBindingPoint() {
  // todo: impl
  return -1;
}

}  // namespace animax
}  // namespace lynx
