// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_CUSTOM_ANDROID_VIDEO_SHADER_YUV_ANDROID_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_ANDROID_VIDEO_SHADER_YUV_ANDROID_H_

#include "include/base/macros.h"
#include "src/video/video_shader_gl.h"

namespace lynx {
namespace animax {
class ANIMAX_EXPORT VideoShaderYUVAndroid : public VideoShaderGL {
 public:
  VideoShaderYUVAndroid() = default;
  ~VideoShaderYUVAndroid() override;
  const char *GetVertexShader() const override;
  const char *GetFragmentShader() const override;
  GLenum GetVideoTextureTarget() override;
  GLenum GetVideoTextureBindingPoint() override;
  void Draw(std::unique_ptr<TextureInfo> texture_info,
            const std::array<float, 16> &transform) override;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_ANDROID_VIDEO_SHADER_YUV_ANDROID_H_
