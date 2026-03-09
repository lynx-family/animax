// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/android/video_shader_yuv_android.h"

#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/base/log/log.h"
#include "src/render/image_gl.h"
#include "src/render/texture_info_frame_data.h"
#include "src/video/custom/yuv_frame_info.h"

namespace lynx {
namespace animax {

VideoShaderYUVAndroid::~VideoShaderYUVAndroid() noexcept {
  if (texture_ != 0) {
    glDeleteTextures(1, &texture_);
  }
  if (!textures_.empty()) {
    glDeleteTextures(textures_.size(), textures_.data());
  }
}

const char *VideoShaderYUVAndroid::GetVertexShader() const {
  static const char *vs = R"(#version 300 es
precision highp float;
precision mediump int;

out vec2 textureOut;

void main() {
    const vec2[] pos = vec2[4](
        vec2(-1.0, -1.0),
        vec2(1.0, -1.0),
        vec2(1.0, 1.0),
        vec2(-1.0, 1.0)
    );
    textureOut = vec2(0.5, 0.5) * (pos[gl_VertexID] + vec2(1.0, 1.0));
    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
}

)";
  return vs;
}

const char *VideoShaderYUVAndroid::GetFragmentShader() const {
  const char *fragment = R"(#version 300 es
precision highp float;

out vec4 fragColor;
in vec2 textureOut;

uniform sampler2D textureY;
uniform sampler2D textureU;
uniform sampler2D textureV;
uniform vec4 rgbFrame;
uniform vec4 aFrame;

const mat3 YUV_TO_RGB_MATRIX = mat3(
    1.1643835616,  0.0000000000,  1.7927410714,
    1.1643835616, -0.2132486143, -0.5329093286,
    1.1643835616,  2.1124017857,  0.0000000000
);

const vec3 YUV_OFFSET = vec3(-0.9729450750, 0.3014826655, -1.1334022179);

vec3 yuvToRgb(float y, float u, float v) {
    return vec3(y, u, v) * YUV_TO_RGB_MATRIX + YUV_OFFSET;
}

float yuvToR(float y, float u, float v) {
    return dot(vec3(y, u, v), YUV_TO_RGB_MATRIX[0]) + YUV_OFFSET[0];
}

vec2 calculateTexCoord(vec2 baseCoord, vec4 frame) {
    return vec2(
        baseCoord.x * frame.z + frame.x,
        baseCoord.y * frame.w + frame.y
    );
}

void main() {
    vec2 rgbCoord = calculateTexCoord(textureOut, rgbFrame);
    vec2 alphaCoord = calculateTexCoord(textureOut, aFrame);

    float y = texture(textureY, rgbCoord).r;
    float u = texture(textureU, rgbCoord).r;
    float v = texture(textureV, rgbCoord).r;

    float ay = texture(textureY, alphaCoord).r;
    float au = texture(textureU, alphaCoord).r;
    float av = texture(textureV, alphaCoord).r;

    vec3 rgbColor = yuvToRgb(y, u, v);
    float alpha = yuvToR(ay, au, av);

    fragColor = vec4(rgbColor, alpha);
}
)";
  return fragment;
}

GLenum VideoShaderYUVAndroid::GetVideoTextureTarget() { return GL_TEXTURE_2D; }

GLenum VideoShaderYUVAndroid::GetVideoTextureBindingPoint() {
  return GL_TEXTURE_BINDING_2D;
}

void VideoShaderYUVAndroid::Draw(std::unique_ptr<TextureInfo> texture_info,
                                 const std::array<float, 16> &transform) {
  if (!Valid() || !texture_info) {
    return;
  }

  auto *texture_frame_data =
      static_cast<TextureInfoFrameData *>(texture_info.get());
  auto frame_info = texture_frame_data->GetFrameInfo();

  DCHECK(GetVideoTextureTarget() == texture_frame_data->Target());

  lynx::animax::ScopedGLResetRestore s0(GL_ACTIVE_TEXTURE);
  {
    lynx::animax::ScopedGLResetRestore s1(GL_FRAMEBUFFER_BINDING);
    lynx::animax::ScopedGLResetRestore s2(GL_CURRENT_PROGRAM);
    lynx::animax::ScopedGLResetRestore s3(GetVideoTextureBindingPoint());
    lynx::animax::ScopedGLResetRestore s4(GL_DEPTH_TEST);
    lynx::animax::ScopedGLResetRestore s5(GL_SCISSOR_TEST);
    lynx::animax::ScopedGLResetRestore s6(GL_VIEWPORT);
    lynx::animax::ScopedGLResetRestore s7(GL_COLOR_CLEAR_VALUE);

    if (!UpdateTexturesFromYuvFrame(frame_info)) {
      return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glUseProgram(program_);

    const GLint texture_units[YUVFrameInfo::kYUVChannels] = {0, 1, 2};
    const char *texture_names[YUVFrameInfo::kYUVChannels] = {
        "textureY", "textureU", "textureV"};

    for (int i = 0; i < YUVFrameInfo::kYUVChannels; ++i) {
      GLint location = glGetUniformLocation(program_, texture_names[i]);
      if (location == -1) {
        continue;
      }
      glActiveTexture(GL_TEXTURE0 + texture_units[i]);
      glBindTexture(GL_TEXTURE_2D, textures_[i]);
      glUniform1i(location, texture_units[i]);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, w_, h_);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
}

bool VideoShaderYUVAndroid::UpdateTexturesFromYuvFrame(
    const std::shared_ptr<YUVFrameInfo> &frame_info) {
  if (!Valid() || !frame_info) {
    return false;
  }

  if (frame_info->GetWidth() == 0 || frame_info->GetHeight() == 0) {
    ANIMAX_LOGE("VideoShaderYUVAndroid: invalid frame is provided.");
    return false;
  }

  bool needs_init = textures_.empty();
  if (needs_init) {
    textures_.resize(YUVFrameInfo::kYUVChannels);
    glGenTextures(YUVFrameInfo::kYUVChannels, textures_.data());
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      ANIMAX_LOGE("VideoShaderYUVAndroid: Failed to gen textures.");
      return false;
    }
  }

  bool success = true;
  for (int ch = 0; ch < YUVFrameInfo::kYUVChannels; ch++) {
    if (textures_[ch] == 0) {
      success = false;
      break;
    }

    auto width = frame_info->GetChannelDataWidth(ch);
    auto height = frame_info->GetChannelDataHeight(ch);
    auto data = frame_info->GetChannelData(ch)->data();

    glBindTexture(GL_TEXTURE_2D, textures_[ch]);
    if (needs_init) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED,
                   GL_UNSIGNED_BYTE, data);
    } else {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
                      GL_UNSIGNED_BYTE, data);
    }
  }

  return success;
}

}  // namespace animax
}  // namespace lynx
