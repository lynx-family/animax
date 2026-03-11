// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
// Copyright 2018 Airbnb, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ANIMAX_SRC_LAYER_SOLID_LAYER_H_
#define ANIMAX_SRC_LAYER_SOLID_LAYER_H_

#include "src/content/effect/blur_element.h"
#include "src/layer/base_layer.h"
#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

class SolidLayer : public BaseLayer {
 public:
  SolidLayer(LayerModel& layer, CompositionModel& composition);

  void Init() override;
  void DrawLayer(Canvas& canvas, Matrix& matrix, int32_t alpha) override;
  void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                 bool apply_parent) override;

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

 private:
  RectF rect_;
  std::unique_ptr<Paint> paint_;
  std::unique_ptr<Path> path_;

  std::unique_ptr<ColorFilterKeyframeAnimation> color_filter_animation_;
  std::unique_ptr<BlurElement> blur_element_;

  float points_[8];
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_SOLID_LAYER_H_
