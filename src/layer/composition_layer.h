// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_COMPOSITION_LAYER_H_
#define ANIMAX_SRC_LAYER_COMPOSITION_LAYER_H_

#include <memory>
#include <vector>

#include "src/layer/base_layer.h"
#include "src/layer/camera_layer.h"
#include "src/model/value/base_value.h"
#include "src/render/camera.h"

namespace lynx {
namespace animax {

class CompositionModel;
class LayerModel;

class CompositionLayer : public BaseLayer {
 public:
  CompositionLayer(LayerModel& layer_model, CompositionModel& composition);
  ~CompositionLayer() override = default;

  void Init() override;

  void DrawLayer(Canvas& canvas, Matrix& matrix, int32_t alpha) override;

  void SetProgress(float progress) override;

  void SetLayerModels(LayerModelList& layers) {
    layer_models_.clear();
    for (auto& layer : layers) {
      layer_models_.push_back(layer.get());
    }
  }

  void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                 bool apply_parent) override;

  void ResolveChildKeyPath(const AnimaXKeyPath& path, int32_t depth,
                           ResolvedKeyPathElements& match_elements,
                           const AnimaXKeyPath& current_partial_path) override;

  void HitTest(float x, float y,
               std::unordered_set<std::string>& hit_layers) override;

  bool GetBoundsByName(const std::string& layer_name,
                       LayerBoundsSpace bounds_space,
                       RectF& out_bounds) override;

 private:
  void DrawLayerInner(Canvas& canvas, Matrix& matrix, int32_t parent_alpha);

  std::unique_ptr<Matrix> GetCameraMatrix(Canvas& canvas);

  std::vector<std::unique_ptr<BaseLayer>> layers_;
  std::vector<std::unique_ptr<BaseLayer>> matte_target_layers_;
  std::vector<LayerModel*> layer_models_;

  RectF rect_;
  RectF new_clip_rect_;
  std::unique_ptr<Paint> layer_paint_;

  std::unique_ptr<FloatKeyframeAnimation> time_remapping_animation_;

  [[maybe_unused]] bool has_masks_ = false;
  bool clip_to_composition_bounds_ = true;

  std::unique_ptr<CameraLayer> camera_layer_;
  std::unique_ptr<Camera> camera_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_COMPOSITION_LAYER_H_
