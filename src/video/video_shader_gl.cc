// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/video_shader_gl.h"

#include "src/base/gl/gl_util.h"
#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/render/image_gl.h"
#include "src/render/texture_info_gl.h"

namespace lynx {
namespace animax {

VideoShaderGL::~VideoShaderGL() {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  if (program_) {
    glDeleteProgram(program_);
  }
  if (fbo_) {
    glDeleteFramebuffers(1, &fbo_);
  }
}

bool VideoShaderGL::Valid() { return !!program_ && !!fbo_ && !!texture_; }

void VideoShaderGL::Init(int32_t w, int32_t h,
                         const std::array<float, 4> &rgb_frame,
                         const std::array<float, 4> &a_frame) {
  if (program_ || fbo_ || texture_) {
    ANIMAX_LOGE("VideoShader is in an incomplete state, program: "
                << !!program_ << ", fbo: " << !!fbo_
                << ", texture: " << !!texture_);
    return;
  }

  w_ = w;
  h_ = h;
  rgb_frame_ = rgb_frame;
  a_frame_ = a_frame;
  InitProgram();
  InitFramebuffer();
}

void VideoShaderGL::InitProgram() {
  program_ = CreateProgram(GetVertexShader(), GetFragmentShader());
  if (!program_) {
    return;
  }
  lynx::animax::ScopedGLResetRestore s(GL_CURRENT_PROGRAM);
  glUseProgram(program_);

  GLint rgb_frame_location = GetRGBFrameLocation();
  if (rgb_frame_location >= 0) {
    glUniform4fv(rgb_frame_location, 1, rgb_frame_.data());
  }

  GLint a_frame_location = GetAFrameLocation();
  if (a_frame_location >= 0) {
    glUniform4fv(a_frame_location, 1, a_frame_.data());
  }
}

void VideoShaderGL::InitFramebuffer() {
  texture_ = OnInitTexture();

  lynx::animax::ScopedGLResetRestore s1(GL_FRAMEBUFFER_BINDING);

  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texture_, 0);
}

GLint VideoShaderGL::GetVideoTextureLocation() {
  if (-1 == video_texture_loc_) {
    video_texture_loc_ = glGetUniformLocation(program_, "video");
  }
  return video_texture_loc_;
}

GLint VideoShaderGL::GetRGBFrameLocation() {
  if (-1 == rgb_frame_loc_) {
    rgb_frame_loc_ = glGetUniformLocation(program_, "rgbFrame");
  }
  return rgb_frame_loc_;
}

GLint VideoShaderGL::GetAFrameLocation() {
  if (-1 == a_frame_loc_) {
    a_frame_loc_ = glGetUniformLocation(program_, "aFrame");
  }
  return a_frame_loc_;
}

void VideoShaderGL::Draw(std::unique_ptr<TextureInfo> texture_info,
                         const std::array<float, 16> &transform) {
  if (!Valid() || !texture_info) {
    return;
  }

  auto *texture_gl = static_cast<TextureInfoGL *>(texture_info.get());
  if (!texture_gl) {
    return;
  }

  GLenum target = texture_gl->Target();
  GLuint texture = texture_gl->ID();

  DCHECK(GetVideoTextureTarget() == target);
  lynx::animax::ScopedGLResetRestore s0(GL_ACTIVE_TEXTURE);
  {
    lynx::animax::ScopedGLResetRestore s1(GL_FRAMEBUFFER_BINDING);
    lynx::animax::ScopedGLResetRestore s2(GL_CURRENT_PROGRAM);
    lynx::animax::ScopedGLResetRestore s3(GetVideoTextureBindingPoint());
    lynx::animax::ScopedGLResetRestore s4(GL_DEPTH_TEST);
    lynx::animax::ScopedGLResetRestore s5(GL_SCISSOR_TEST);
    lynx::animax::ScopedGLResetRestore s6(GL_VIEWPORT);
    lynx::animax::ScopedGLResetRestore s7(GL_COLOR_CLEAR_VALUE);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glUseProgram(program_);
    GLint transform_location = GetTransformMatrixLocation();
    if (transform_location >= 0) {
      glUniformMatrix4fv(transform_location, 1, false, transform.data());
    }
    GLint video_texture_location = GetVideoTextureLocation();
    if (video_texture_location >= 0) {
      glUniform1i(video_texture_location, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(target, texture);
    }
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, w_, h_);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
}

std::unique_ptr<Image> VideoShaderGL::GetOutputImage(RealContext *context) {
  if (texture_ == 0) {
    return std::unique_ptr<Image>();
  }

#if !defined(OS_WIN)
  if (context->GetBackendType() != ContextBackend::kOpenGL) {
    return std::unique_ptr<Image>();
  }
#endif

  TextureInfoGL info(texture_, w_, h_, GL_TEXTURE_2D);
  return std::make_unique<ImageGL>(&info, context);
}

GLuint VideoShaderGL::OnInitTexture() {
  lynx::animax::ScopedGLResetRestore s0(GL_TEXTURE_BINDING_2D);
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  std::vector<uint8_t> zeros(w_ * h_ * 4, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               zeros.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return tex;
}

}  // namespace animax
}  // namespace lynx
