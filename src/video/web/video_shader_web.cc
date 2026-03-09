// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/web/video_shader_web.h"

#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/render/texture_info_gl.h"

namespace lynx {
namespace animax {

VideoShaderWeb::~VideoShaderWeb() {}

std::unique_ptr<VideoShader> VideoShader::Make(
    const AnimaXAbility *ability_ptr) {
  return std::unique_ptr<VideoShaderWeb>(new VideoShaderWeb());
}

// TODO(aiyongbiao.rick): Implement VideoShaderWeb methods.

const char *VideoShaderWeb::GetVertexShader() const { return nullptr; }

const char *VideoShaderWeb::GetFragmentShader() const { return nullptr; }

GLint VideoShaderWeb::GetTransformMatrixLocation() { return -1; }

GLenum VideoShaderWeb::GetVideoTextureTarget() { return -1; }

GLenum VideoShaderWeb::GetVideoTextureBindingPoint() { return -1; }

}  // namespace animax
}  // namespace lynx
