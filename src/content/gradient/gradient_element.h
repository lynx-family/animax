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

#ifndef ANIMAX_SRC_CONTENT_GRADIENT_GRADIENT_ELEMENT_H_
#define ANIMAX_SRC_CONTENT_GRADIENT_GRADIENT_ELEMENT_H_

#include <unordered_map>

#include "src/animation/base_keyframe_animation.h"
#include "src/model/animatable/animatable_value.h"
#include "src/model/animatable/base_animatable_value.h"
#include "src/model/value/gradient_color.h"
#include "src/render/matrix.h"
#include "src/render/paint.h"
#include "src/render/shader.h"

namespace lynx {
namespace animax {

class BaseLayer;

class GradientElement : public AnimationListener {
 public:
  GradientElement(BaseLayer& layer, AnimatableGradientValue& gradient_color,
                  AnimatablePointValue& start_point,
                  AnimatablePointValue& end_point, GradientType type,
                  AnimatableFloatValue* highlight_length,
                  AnimatableFloatValue* highlight_angle, bool keep_min_radius);

  void Init();
  void Draw(Paint& paint, Matrix& parent_matrix);

  void OnValueChanged() override {}

 private:
  std::unique_ptr<Shader> GetLinearGradient(Matrix& matrix);
  std::unique_ptr<Shader> GetRadialGradient(Matrix& matrix);
  int32_t GetGradientHash();
  int32_t* ApplyDynamicColorIfNeeds(int32_t* colors);

  GradientType type_;
  bool keep_min_radius_ = true;

  std::unordered_map<int32_t, std::unique_ptr<Shader>> linear_gradient_cache_;
  std::unordered_map<int32_t, std::unique_ptr<Shader>> radial_gradient_cache_;

  std::unique_ptr<GradientKeyframeAnimation> color_animation_;
  std::unique_ptr<PointKeyframeAnimation> start_point_animation_;
  std::unique_ptr<PointKeyframeAnimation> end_point_animation_;
  std::unique_ptr<FloatKeyframeAnimation> highlight_length_animation_;
  std::unique_ptr<FloatKeyframeAnimation> highlight_angle_animation_;
  int32_t cache_steps_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_GRADIENT_GRADIENT_ELEMENT_H_
