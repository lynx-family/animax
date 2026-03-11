// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/harmony/video_shader_harmony.h"

#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/render/texture_info_gl.h"

namespace lynx {
namespace animax {

VideoShaderHarmony::~VideoShaderHarmony() noexcept {
  if (texture_) {
    // Harmony use pure OpenGLES and need to delete this texture handler
    glDeleteTextures(1, &texture_);
  }
}

std::unique_ptr<VideoShader> VideoShader::Make() {
  return std::unique_ptr<VideoShaderHarmony>(new VideoShaderHarmony());
}

const char *VideoShaderHarmony::GetVertexShader() const {
  static const char *vs = R"(#version 300 es
precision highp float;
precision mediump int;

out vec2 tc;

void main() {
    const vec2[] pos = vec2[4](
        vec2(-1.0, -1.0),
        vec2(1.0, -1.0),
        vec2(1.0, 1.0),
        vec2(-1.0, 1.0)
    );
    tc = vec2(0.5, 0.5) * (pos[gl_VertexID] + vec2(1.0, 1.0));
    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
}

)";
  return vs;
}

const char *VideoShaderHarmony::GetFragmentShader() const {
  static const char *fs = R"(#version 300 es
#extension GL_OES_EGL_image_external: require
#extension GL_OES_EGL_image_external_essl3 : enable
precision highp float;
precision mediump int;

in vec2 tc;
uniform samplerExternalOES video;
uniform highp mat4 transformMatrix;
uniform vec4 rgbFrame;
uniform vec4 aFrame;

out vec4 color;
void main() {
    vec2 alphaFrameCoord = vec2(tc.x * aFrame.z + aFrame.x, tc.y * aFrame.w + aFrame.y);
    alphaFrameCoord = (transformMatrix * vec4(alphaFrameCoord.x, 1.0 - alphaFrameCoord.y, 0.0, 1.0)).xy;
    float a = texture(video, alphaFrameCoord).r;
    vec2 rgbFrameCoord = vec2(tc.x * rgbFrame.z + rgbFrame.x, tc.y * rgbFrame.w + rgbFrame.y);
    rgbFrameCoord = (transformMatrix * vec4(rgbFrameCoord.x, 1.0 - rgbFrameCoord.y, 0.0, 1.0)).xy;
    color = vec4(texture(video, rgbFrameCoord).rgb, a);
}
)";
  return fs;
}

GLint VideoShaderHarmony::GetTransformMatrixLocation() {
  if (-1 == transform_matrix_loc_) {
    transform_matrix_loc_ = glGetUniformLocation(program_, "transformMatrix");
  }
  return transform_matrix_loc_;
}

GLenum VideoShaderHarmony::GetVideoTextureTarget() {
  return GL_TEXTURE_EXTERNAL_OES;
}

GLenum VideoShaderHarmony::GetVideoTextureBindingPoint() {
  return GL_TEXTURE_BINDING_EXTERNAL_OES;
}

}  // namespace animax
}  // namespace lynx
