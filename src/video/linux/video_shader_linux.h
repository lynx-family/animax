// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_LINUX_VIDEO_SHADER_LINUX_H_
#define ANIMAX_SRC_VIDEO_LINUX_VIDEO_SHADER_LINUX_H_

#include "base/include/fml/task_runner.h"
#include "src/video/video_shader.h"

namespace lynx {
namespace animax {

class VideoShaderLinux final : public VideoShader {
 public:
  explicit VideoShaderLinux(std::shared_ptr<AnimaXAbility> ability);
  ~VideoShaderLinux() override = default;
  bool Valid() override;
  void Init(int32_t w, int32_t h, const std::array<float, 4>& rgb_frame,
            const std::array<float, 4>& a_frame) override;
  void Draw(std::unique_ptr<TextureInfo> texture_info,
            const std::array<float, 16>& transform) override;
  std::unique_ptr<Image> GetOutputImage(RealContext* context) override;

 private:
  class State;
  fml::RefPtr<fml::TaskRunner> raster_runner_;
  std::shared_ptr<State> state_;
  int32_t width_ = 0;
  int32_t height_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_LINUX_VIDEO_SHADER_LINUX_H_
