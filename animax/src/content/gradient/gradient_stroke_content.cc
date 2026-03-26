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

#include "src/content/gradient/gradient_stroke_content.h"

#include "src/content/shape/base_stroke_content.h"
#include "src/model/gradient/gradient_stroke_model.h"

namespace lynx {
namespace animax {

GradientStrokeContent::GradientStrokeContent(BaseLayer& layer,
                                             GradientStrokeModel& model)
    : BaseStrokeContent(layer, ToPaintCap(model.cap_type_),
                        ToPaintJoin(model.join_type_), model.miter_limit_,
                        model.opacity_, model.width_, model.line_dash_pattern_,
                        model.dash_offset_) {
  name_ = model.name_;
  hidden_ = model.hidden_;

  if (model.gradient_color_ && model.start_point_ && model.end_point_) {
    gradient_element_ = std::make_unique<GradientElement>(
        layer, *model.gradient_color_, *model.start_point_, *model.end_point_,
        model.gradient_type_, model.highlight_length_.get(),
        model.highlight_angle_.get(), false);
  }
}

void GradientStrokeContent::Init() {
  BaseStrokeContent::Init();

  if (gradient_element_) {
    gradient_element_->Init();
  }
}

void GradientStrokeContent::Draw(Canvas& canvas, Matrix& parent_matrix,
                                 int32_t parent_alpha) {
  if (hidden_) {
    return;
  }

  GetBounds(bounds_rect_, parent_matrix, false);

  if (gradient_element_) {
    gradient_element_->Draw(*paint_, parent_matrix);
  }

  BaseStrokeContent::Draw(canvas, parent_matrix, parent_alpha);
}

}  // namespace animax
}  // namespace lynx
