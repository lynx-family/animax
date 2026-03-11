// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_VIDEO_SHADER_GL_H_
#define ANIMAX_SRC_VIDEO_VIDEO_SHADER_GL_H_

#include <array>
#include <memory>

#include "include/base/macros.h"
#include "src/base/gl/gl_include.h"
#include "src/render/image.h"
#include "src/render/real_context.h"
#include "src/video/video_shader.h"

namespace lynx {
namespace animax {
class ANIMAX_EXPORT VideoShaderGL : public VideoShader {
 public:
  VideoShaderGL() = default;
  ~VideoShaderGL() override;

  // VideoShader interface implementation
  bool Valid() override;
  void Init(int32_t w, int32_t h, const std::array<float, 4> &rgb_frame,
            const std::array<float, 4> &a_frame) override;
  void Draw(std::unique_ptr<TextureInfo> texture_info,
            const std::array<float, 16> &transform) override;
  std::unique_ptr<Image> GetOutputImage(RealContext *context) override;
  GLint GetVideoTextureLocation();
  GLint GetRGBFrameLocation();
  GLint GetAFrameLocation();

  virtual const char *GetVertexShader() const = 0;
  virtual const char *GetFragmentShader() const = 0;
  virtual GLint GetTransformMatrixLocation() { return -1; }
  virtual GLenum GetVideoTextureTarget() = 0;
  virtual GLenum GetVideoTextureBindingPoint() = 0;

 protected:
  void InitProgram();
  void InitFramebuffer();
  GLuint OnInitTexture();

  GLuint program_ = 0;
  GLuint fbo_ = 0;
  GLuint texture_ = 0;
  GLint video_texture_loc_ = -1;
  GLint rgb_frame_loc_ = -1;
  GLint a_frame_loc_ = -1;

  int32_t w_ = 0;
  int32_t h_ = 0;
  std::array<float, 4> rgb_frame_;
  std::array<float, 4> a_frame_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_VIDEO_SHADER_GL_H_
