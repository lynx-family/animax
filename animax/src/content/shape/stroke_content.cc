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

#include "src/content/shape/stroke_content.h"

#include "src/model/shape/shape_stroke_model.h"

namespace lynx {
namespace animax {

StrokeContent::StrokeContent(BaseLayer& layer, ShapeStrokeModel& model)
    : BaseStrokeContent(layer, model.cap_type_, model.join_type_,
                        model.miter_limit_, model.opacity_, model.width_,
                        model.line_dash_pattern_, model.offset_),
      layer_(layer) {
  name_ = model.name_;
  hidden_ = model.hidden_;

  color_animation_ = model.color_->CreateAnimation();
  if (color_animation_) {
    layer_.AddAnimation(color_animation_.get());
  }
}

void StrokeContent::Init() {
  BaseStrokeContent::Init();

  if (color_animation_) {
    color_animation_->AddUpdateListener(this);
  }
}

void StrokeContent::Draw(Canvas& canvas, Matrix& parent_matrix,
                         int32_t parent_alpha) {
  if (hidden_) {
    return;
  }

  paint_->SetColor(color_animation_->GetValue());

  if (color_filter_animation_) {
    paint_->SetColorFilter(color_filter_animation_->GetValue());
  }

  BaseStrokeContent::Draw(canvas, parent_matrix, parent_alpha);
}

KeyframeAnimation* StrokeContent::GetAnimationForProperty(
    LayerPropertyType type) {
  switch (type) {
    case LayerPropertyType::kStrokeColor: {
      return GetOrCreateAnimation<ColorKeyframeAnimation>(color_animation_,
                                                          layer_, this);
    }
    case LayerPropertyType::kColorFilter: {
      return GetOrCreateAnimation<ColorFilterKeyframeAnimation>(
          color_filter_animation_, layer_, this);
    }
    default: {
      return BaseStrokeContent::GetAnimationForProperty(type);
    }
  }
}
}  // namespace animax
}  // namespace lynx
