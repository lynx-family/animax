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

#include "src/model/layer_model.h"

#include "src/model/animatable/animatable_text_properties.h"
#include "src/model/composition_model.h"
#include "src/model/effect/blur_effect_model.h"
#include "src/model/effect/drop_shadow_effect_model.h"
#include "src/model/text/text_gradient_model.h"

namespace lynx {
namespace animax {

std::unique_ptr<LayerModel> LayerModel::Make(CompositionModel& composition) {
  return std::unique_ptr<LayerModel>(new LayerModel(composition));
}

LayerModel::LayerModel(CompositionModel& composition)
    : composition_(composition) {}

LayerModel::~LayerModel() = default;

void LayerModel::Init(
    std::string layer_name, LayerType layer_type, int32_t layer_id,
    int32_t parent_id, std::string ref_id, int32_t solid_width,
    int32_t solid_height, std::unique_ptr<Color> solid_color,
    float time_stretch, float start_frame, float pre_comp_width,
    float pre_comp_height, std::unique_ptr<AnimatableTextFrame> text,
    std::unique_ptr<AnimatableTextPropertyList> text_properties_list,
    bool hidden, std::unique_ptr<AnimatableTransformModel> transform,
    int32_t matte_type_int, int32_t matte_parent, bool is_matte_target,
    std::unique_ptr<AnimatableFloatValue> time_remapping,
    std::unique_ptr<BlurEffectModel> blur_effect,
    std::unique_ptr<DropShadowEffectModel> drop_shadow_effect, bool enable_3d,
    std::unique_ptr<AnimatableFloatValue> perspective,
    BlendModeType blend_mode) {
  layer_name_ = std::move(layer_name);
  layer_type_ = layer_type;
  layer_id_ = layer_id;
  parent_id_ = parent_id;
  ref_id_ = std::move(ref_id);
  solid_width_ = solid_width;
  solid_height_ = solid_height;
  solid_color_ = std::move(solid_color);
  time_stretch_ = time_stretch;
  start_frame_ = start_frame;
  pre_comp_width_ = pre_comp_width;
  pre_comp_height_ = pre_comp_height;
  text_ = std::move(text);
  text_properties_list_ = std::move(text_properties_list);
  hidden_ = hidden;
  transform_ = std::move(transform);
  matte_type_ = static_cast<MatteType>(matte_type_int);
  matte_parent_ = matte_parent;
  is_matte_target_ = is_matte_target;
  time_remapping_ = std::move(time_remapping);
  blur_effect_ = std::move(blur_effect);
  drop_shadow_effect_ = std::move(drop_shadow_effect);
  enable_3d_ = enable_3d;
  perspective_ = std::move(perspective);
  blend_mode_ = blend_mode;
}

AnimatableTransformModel* LayerModel::GetTransform() {
  return transform_.get();
}

void LayerModel::SetTextGradientModel(
    std::unique_ptr<TextGradientModel> model) {
  if (!model) {
    return;
  }
  if (!text_gradient_model_) {
    text_gradient_model_ = std::move(model);
    return;
  }
  text_gradient_model_->Append(std::move(*model));
}

const TextGradientModel* LayerModel::GetTextGradientModel() const {
  return text_gradient_model_.get();
}

float LayerModel::GetStartProgress() {
  return start_frame_ / composition_.GetDurationFrames();
}

}  // namespace animax
}  // namespace lynx
