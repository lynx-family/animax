// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_HARMONY_VIDEO_SHADER_HARMONY_H_
#define ANIMAX_SRC_VIDEO_HARMONY_VIDEO_SHADER_HARMONY_H_

#include "src/video/video_shader_gl.h"

namespace lynx {
namespace animax {

class VideoShaderHarmony : public VideoShaderGL {
 public:
  VideoShaderHarmony() = default;
  ~VideoShaderHarmony() noexcept override;
  const char *GetVertexShader() const override;
  const char *GetFragmentShader() const override;
  GLint GetTransformMatrixLocation() override;
  GLenum GetVideoTextureTarget() override;
  GLenum GetVideoTextureBindingPoint() override;

 private:
  GLint transform_matrix_loc_ = -1;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_HARMONY_VIDEO_SHADER_HARMONY_H_
