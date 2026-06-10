// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_DARWIN_VIDEO_SHADER_DARWIN_H_
#define ANIMAX_SRC_VIDEO_DARWIN_VIDEO_SHADER_DARWIN_H_

#import <CoreVideo/CoreVideo.h>
#import <Metal/Metal.h>

#include <array>
#include <memory>

#include "src/render/image.h"
#include "src/render/real_context.h"
#include "src/video/video_shader.h"

namespace lynx {
namespace animax {

class VideoShaderDarwin : public VideoShader {
 public:
  VideoShaderDarwin();
  ~VideoShaderDarwin() = default;

  // VideoShader interface implementation
  bool Valid() override;
  void Init(int32_t w, int32_t h, const std::array<float, 4> &rgb_frame,
            const std::array<float, 4> &a_frame) override;
  void Draw(std::unique_ptr<TextureInfo> texture_info,
            const std::array<float, 16> &transform) override;

  std::unique_ptr<Image> GetOutputImage(RealContext *context) override;

 private:
  bool InitMetalResources();
  bool SetupVertexData();
  bool CreateOutputTexture();

  id<MTLDevice> mtl_device_;
  id<MTLRenderPipelineState> pipeline_state_;
  id<MTLBuffer> position_factor_;

  id<MTLCommandQueue> command_queue_;
  id<MTLBuffer> vertex_buffer_;
  id<MTLTexture> output_texture_;

  int32_t w_;
  int32_t h_;
  std::array<float, 4> rgb_frame_;
  std::array<float, 4> a_frame_;
  bool is_valid_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_DARWIN_VIDEO_SHADER_DARWIN_H_
