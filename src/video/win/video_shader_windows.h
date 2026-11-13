// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_VIDEO_WIN_VIDEO_SHADER_WINDOWS_H_
#define ANIMAX_SRC_VIDEO_WIN_VIDEO_SHADER_WINDOWS_H_

#include "base/include/closure.h"
#include "base/include/fml/task_runner.h"
#include "src/player/win/animax_ability_windows.h"
#include "src/video/video_shader.h"

namespace lynx {
namespace animax {

class VideoShaderYUV;

class VideoShaderWindows : public VideoShader {
 public:
  explicit VideoShaderWindows(std::shared_ptr<AnimaXAbility> ability);
  ~VideoShaderWindows() override;
  bool Valid() override;
  void Init(int32_t w, int32_t h, const std::array<float, 4> &rgb_frame,
            const std::array<float, 4> &a_frame) override;
  void Draw(std::unique_ptr<TextureInfo> texture_info,
            const std::array<float, 16> &transform) override;
  std::unique_ptr<Image> GetOutputImage(RealContext *context) override;

 private:
  struct RasterState;
  void PostToRasterThread(base::MoveOnlyClosure<void, VideoShaderYUV &> task);

  fml::RefPtr<fml::TaskRunner> raster_runner_;
  std::weak_ptr<AnimaXPlayer> weak_player_;
  std::shared_ptr<RasterState> raster_state_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_WIN_VIDEO_SHADER_WINDOWS_H_
