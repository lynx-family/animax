// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_SHADER_YUV_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_SHADER_YUV_H_

#include <vector>

#include "include/base/macros.h"
#include "src/video/video_shader_gl.h"

namespace lynx {
namespace animax {

class YUVFrameInfo;
class ANIMAX_EXPORT VideoShaderYUV : public VideoShaderGL {
 public:
  VideoShaderYUV();
  ~VideoShaderYUV() override;
  const char *GetVertexShader() const override;
  const char *GetFragmentShader() const override;
  GLenum GetVideoTextureTarget() override;
  GLenum GetVideoTextureBindingPoint() override;
  void Draw(std::unique_ptr<TextureInfo> texture_info,
            const std::array<float, 16> &transform) override;

 protected:
  bool UpdateTexturesFromYuvFrame(
      const std::shared_ptr<YUVFrameInfo> &frame_info);
  std::vector<GLuint> textures_;
  std::vector<GLint> texture_locs_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_VIDEO_SHADER_YUV_H_
