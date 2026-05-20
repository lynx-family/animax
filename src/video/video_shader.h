// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_VIDEO_SHADER_H_
#define ANIMAX_SRC_VIDEO_VIDEO_SHADER_H_

#include <array>
#include <memory>

#include "src/player/animax_ability.h"
#include "src/render/texture_info.h"

namespace lynx {
namespace animax {

class Image;
class RealContext;

class VideoShader {
 public:
  static std::unique_ptr<VideoShader> Make(
      std::shared_ptr<AnimaXAbility> ability);
  VideoShader() = default;
  virtual ~VideoShader() = default;

  /**
   * Whether the VideoShader is valid.
   * Normally, after calling Init, the video shader becomes valid. If video
   * shader is valid, you can call Draw and OutputTexture.
   * @return true if the video shader is valid.
   */
  virtual bool Valid() = 0;
  /**
   * Initialize the VideoShader.
   * @param w         the width of output texture.
   * @param h         the height of output texture.
   * @param rgb_frame the position and size of rgb part in video texture. 1st
   * value means x offset, 2nd value means y offset, 3rd value means width of
   * rgb part, 4th value means height of rgb part.
   * @param a_frame   the position and size of alpha part in video texture. 1st
   * value means x offset, 2nd value means y offset, 3rd value means width of
   * alpha part, 4th value means height of alpha part.
   */
  virtual void Init(int32_t w, int32_t h, const std::array<float, 4> &rgb_frame,
                    const std::array<float, 4> &a_frame) = 0;
  /**
   * Compose rgb part and alpha part of video texture into a composite texute.
   * After calling this, you can get result composite texture through
   * OutputTexture.
   * @param texture_info the input video texture info
   * @param transform    the transform applying to the texture
   */
  virtual void Draw(std::unique_ptr<TextureInfo> texture_info,
                    const std::array<float, 16> &transform) = 0;
  /**
   * The output image texture of VideoShader. Can be rendered by Canvas.
   * If the video shader is valid, you can get output image texture through
   * this. In the lifecycle of the video shader, it won't change.
   * @return the output texture.
   */
  virtual std::unique_ptr<Image> GetOutputImage(RealContext *context) = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_VIDEO_SHADER_H_
