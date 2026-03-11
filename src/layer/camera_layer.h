// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_CAMERA_LAYER_H_
#define ANIMAX_SRC_LAYER_CAMERA_LAYER_H_

#include "src/layer/base_layer.h"
#include "src/model/value/base_value.h"
#include "src/render/camera.h"

namespace lynx {
namespace animax {
class CameraLayer : public BaseLayer {
 public:
  CameraLayer(LayerModel& layer, CompositionModel& composition);

  void Init() override;

  void DrawLayer(Canvas& canvas, Matrix& parent_matrix,
                 int32_t parent_alpha) override;

  bool ConfigCamera(Camera* camera);

 private:
  std::unique_ptr<FloatKeyframeAnimation> perspective_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_CAMERA_LAYER_H_
