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

#ifndef ANIMAX_SRC_MODEL_GRADIENT_GRADIENT_STROKE_MODEL_H_
#define ANIMAX_SRC_MODEL_GRADIENT_GRADIENT_STROKE_MODEL_H_

#include <memory>
#include <string>
#include <vector>

#include "src/content/gradient/gradient_stroke_content.h"
#include "src/model/animatable/base_animatable_value.h"
#include "src/model/content_model.h"
#include "src/model/shape/shape_stroke_model.h"
#include "src/model/value/base_value.h"
#include "src/model/value/gradient_color.h"

namespace lynx {
namespace animax {

class GradientStrokeModel : public ContentModel {
 public:
  GradientStrokeModel() = default;

  void Init(std::string name, GradientType gradient_type,
            std::unique_ptr<AnimatableGradientValue> gradient_color,
            std::unique_ptr<AnimatableIntegerValue> opacity,
            std::unique_ptr<AnimatablePointValue> start_point,
            std::unique_ptr<AnimatablePointValue> end_point,
            std::unique_ptr<AnimatableFloatValue> highlight_length,
            std::unique_ptr<AnimatableFloatValue> highlight_angle,
            std::unique_ptr<AnimatableFloatValue> width, LineCapType cap_type,
            LineJoinType join_type, float miter_limit,
            std::shared_ptr<AnimatableFloatValue> dash_offset, bool hidden) {
    name_ = std::move(name);
    gradient_type_ = gradient_type;
    gradient_color_ = std::move(gradient_color);
    opacity_ = std::move(opacity);
    start_point_ = std::move(start_point);
    end_point_ = std::move(end_point);
    highlight_length_ = std::move(highlight_length);
    highlight_angle_ = std::move(highlight_angle);
    width_ = std::move(width);
    cap_type_ = cap_type;
    join_type_ = join_type;
    miter_limit_ = miter_limit;
    dash_offset_ = std::move(dash_offset);
    hidden_ = hidden;
  }

  std::vector<std::shared_ptr<AnimatableFloatValue>>& GetDashOffset() {
    return line_dash_pattern_;
  }

  std::unique_ptr<Content> CreateContent(CompositionModel& composition,
                                         BaseLayer& layer) override {
    return std::unique_ptr<GradientStrokeContent>(
        new GradientStrokeContent(layer, *this));
  }

 private:
  friend class GradientStrokeContent;

  GradientType gradient_type_;
  std::unique_ptr<AnimatableGradientValue> gradient_color_;
  std::unique_ptr<AnimatableIntegerValue> opacity_;
  std::unique_ptr<AnimatablePointValue> start_point_;
  std::unique_ptr<AnimatablePointValue> end_point_;
  std::unique_ptr<AnimatableFloatValue> highlight_length_;
  std::unique_ptr<AnimatableFloatValue> highlight_angle_;
  std::unique_ptr<AnimatableFloatValue> width_;
  LineCapType cap_type_;
  LineJoinType join_type_;
  float miter_limit_ = 0;
  std::vector<std::shared_ptr<AnimatableFloatValue>> line_dash_pattern_;
  std::shared_ptr<AnimatableFloatValue> dash_offset_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_GRADIENT_GRADIENT_STROKE_MODEL_H_
