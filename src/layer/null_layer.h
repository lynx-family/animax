// Copyright 2023 The Lynx Authors. All rights reserved.
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

#ifndef ANIMAX_SRC_LAYER_NULL_LAYER_H_
#define ANIMAX_SRC_LAYER_NULL_LAYER_H_

#include "src/layer/base_layer.h"
#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

class NullLayer : public BaseLayer {
 public:
  NullLayer(LayerModel& layer, CompositionModel& composition)
      : BaseLayer(layer, composition) {}

  void DrawLayer(Canvas& canvas, Matrix& matrix, int32_t alpha) override {}

  void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                 bool apply_parent) override {
    BaseLayer::GetBounds(out_bounds, parent_matrix, apply_parent);
    out_bounds.Set(0, 0, 0, 0);
  }
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_NULL_LAYER_H_
