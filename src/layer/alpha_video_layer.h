// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_ALPHA_VIDEO_LAYER_H_
#define ANIMAX_SRC_LAYER_ALPHA_VIDEO_LAYER_H_

#include "src/animation/base_keyframe_animation.h"
#include "src/layer/base_layer.h"
#include "src/model/value/base_value.h"
#include "src/render/image.h"
#include "src/render/paint.h"
#include "src/resource/asset/video_asset.h"
#include "src/video/video_player.h"
#include "src/video/video_shader.h"

namespace lynx {
namespace animax {
class VideoShader;
class VideoPlayer;

class AlphaVideoLayer : public BaseLayer {
 public:
  AlphaVideoLayer(LayerModel& layer_model, CompositionModel& composition);
  ~AlphaVideoLayer() override = default;

  void Init() override;
  void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                 bool apply_parent) override;
  void DrawLayer(Canvas& canvas, Matrix& matrix, int32_t alpha) override;

 private:
  void AttachAssetOnce();
  void UpdateDownsampleSize(int32_t canvas_width, int32_t canvas_height);

  Image* UpdateCompositeImage(RealContext* real_context);
  Image* GetCompositeImage(RealContext* real_context);
  int32_t GetCurrentFrame();

  std::unique_ptr<Paint> paint_;
  float scale_ = 1.0;
  RectF src_;
  RectF dst_;
  int32_t canvas_width_ = 0;
  int32_t canvas_height_ = 0;
  int32_t downsample_width_ = 0;
  int32_t downsample_height_ = 0;
  bool enable_downsample_ = false;

  std::shared_ptr<VideoAsset> video_asset_;
  std::unique_ptr<VideoShader> video_shader_;
  std::unique_ptr<VideoPlayer> video_player_;

  std::unique_ptr<Image> image_;

  bool has_attach_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_ALPHA_VIDEO_LAYER_H_
