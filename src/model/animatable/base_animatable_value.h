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

#ifndef ANIMAX_SRC_MODEL_ANIMATABLE_BASE_ANIMATABLE_VALUE_H_
#define ANIMAX_SRC_MODEL_ANIMATABLE_BASE_ANIMATABLE_VALUE_H_

#include <memory>
#include <vector>

#include "src/model/animatable/animatable_value.h"
#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

class CompositionModel;

class FloatKeyframeAnimation;
class IntegerKeyframeAnimation;
class PointKeyframeAnimation;
class ScaleKeyframeAnimation;
class ColorKeyframeAnimation;
class ShapeKeyframeAnimation;
class GradientKeyframeAnimation;
class OrientationKeyframeAnimation;

class AnimatableFloatValue : public AnimatableValue {
 public:
  std::unique_ptr<FloatKeyframeAnimation> CreateAnimation();
};

class AnimatableIntegerValue : public AnimatableValue {
 public:
  std::unique_ptr<IntegerKeyframeAnimation> CreateAnimation();

  // Creates a default AnimatableIntegerValue with a single keyframe of the
  // given value. Commonly used to provide a default opacity of 100.
  static std::unique_ptr<AnimatableIntegerValue> MakeDefault(
      CompositionModel& composition, int32_t value = 100);
};

class AnimatablePointValue : public AnimatableValue {
 public:
  virtual std::unique_ptr<PointKeyframeAnimation> CreateAnimation(
      bool enable_3d = false);
};

class AnimatableScaleValue : public AnimatableValue {
 public:
  std::unique_ptr<ScaleKeyframeAnimation> CreateAnimation();
};

class AnimatableColorValue : public AnimatableValue {
 public:
  std::unique_ptr<ColorKeyframeAnimation> CreateAnimation();
};

class AnimatablePathValue : public AnimatablePointValue {
 public:
  std::unique_ptr<PointKeyframeAnimation> CreateAnimation(
      bool enable_3d) override;
};

class AnimatableSplitDimensionPathValue : public AnimatablePointValue {
 public:
  AnimatableSplitDimensionPathValue(
      std::unique_ptr<AnimatableFloatValue> anim_x,
      std::unique_ptr<AnimatableFloatValue> anim_y)
      : anim_x_dimen_(std::move(anim_x)), anim_y_dimen_(std::move(anim_y)) {}

  std::unique_ptr<PointKeyframeAnimation> CreateAnimation(
      bool enable_3d) override;

  bool IsStatic() override;
  ValueType Type() override { return ValueType::kSplitPath; }

 private:
  std::unique_ptr<AnimatableFloatValue> anim_x_dimen_;
  std::unique_ptr<AnimatableFloatValue> anim_y_dimen_;
};

class AnimatableShapeValue : public AnimatableValue {
 public:
  bool IsStatic() override { return false; }
  std::unique_ptr<ShapeKeyframeAnimation> CreateAnimation();
};

class AnimatableGradientValue : public AnimatableValue {
 public:
  std::unique_ptr<GradientKeyframeAnimation> CreateAnimation();
};

class AnimatableOrientationValue : public AnimatableValue {
 public:
  std::unique_ptr<OrientationKeyframeAnimation> CreateAnimation();
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_ANIMATABLE_BASE_ANIMATABLE_VALUE_H_
