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

#include "src/model/animatable/base_animatable_value.h"

#include <memory>
#include <vector>

#include "src/animation/base_keyframe_animation.h"
#include "src/animation/shape_keyframe_animation.h"
#include "src/animation/split_dimension_path_keyframe_animation.h"

namespace lynx {
namespace animax {

std::unique_ptr<FloatKeyframeAnimation>
AnimatableFloatValue::CreateAnimation() {
  return std::unique_ptr<FloatKeyframeAnimation>(
      new FloatKeyframeAnimation(frames_));
}

std::unique_ptr<IntegerKeyframeAnimation>
AnimatableIntegerValue::CreateAnimation() {
  return std::unique_ptr<IntegerKeyframeAnimation>(
      new IntegerKeyframeAnimation(frames_));
}

std::unique_ptr<AnimatableIntegerValue> AnimatableIntegerValue::MakeDefault(
    CompositionModel& composition, int32_t value) {
  auto result = std::make_unique<AnimatableIntegerValue>();
  auto& frames = result->GetKeyframes();
  frames.emplace_back(std::make_unique<KeyframeModel>(
      composition, ValueFactory::Make<Integer>(value)));
  return result;
}

std::unique_ptr<PointKeyframeAnimation> AnimatablePointValue::CreateAnimation(
    bool enable_3d) {
  return std::unique_ptr<PointKeyframeAnimation>(
      new PointKeyframeAnimation(frames_));
}

std::unique_ptr<ScaleKeyframeAnimation>
AnimatableScaleValue::CreateAnimation() {
  return std::unique_ptr<ScaleKeyframeAnimation>(
      new ScaleKeyframeAnimation(frames_));
}

std::unique_ptr<ColorKeyframeAnimation>
AnimatableColorValue::CreateAnimation() {
  return std::unique_ptr<ColorKeyframeAnimation>(
      new ColorKeyframeAnimation(frames_));
}

std::unique_ptr<PointKeyframeAnimation> AnimatablePathValue::CreateAnimation(
    bool enable_3d) {
  DCHECK(frames_);
  if (frames_->front()->IsStatic()) {
    return std::unique_ptr<PointKeyframeAnimation>(
        new PointKeyframeAnimation(frames_));
  } else {
    return std::unique_ptr<PathKeyframeAnimation>(
        new PathKeyframeAnimation(frames_, enable_3d));
  }
}

bool AnimatableSplitDimensionPathValue::IsStatic() {
  return anim_x_dimen_->IsStatic() && anim_y_dimen_->IsStatic() &&
         (!anim_z_dimen_ || anim_z_dimen_->IsStatic());
}

std::unique_ptr<PointKeyframeAnimation>
AnimatableSplitDimensionPathValue::CreateAnimation(bool enable_3d) {
  return std::make_unique<SplitDimensionPathKeyframeAnimation>(
      anim_x_dimen_->CreateAnimation(), anim_y_dimen_->CreateAnimation(),
      anim_z_dimen_ ? anim_z_dimen_->CreateAnimation() : nullptr,
      std::make_shared<KeyframeModelList>());
}

std::unique_ptr<ShapeKeyframeAnimation>
AnimatableShapeValue::CreateAnimation() {
  return std::unique_ptr<ShapeKeyframeAnimation>(
      new ShapeKeyframeAnimation(frames_));
}

std::unique_ptr<GradientKeyframeAnimation>
AnimatableGradientValue::CreateAnimation() {
  return std::unique_ptr<GradientKeyframeAnimation>(
      new GradientKeyframeAnimation(frames_));
}

std::unique_ptr<OrientationKeyframeAnimation>
AnimatableOrientationValue::CreateAnimation() {
  return std::unique_ptr<OrientationKeyframeAnimation>(
      new OrientationKeyframeAnimation(frames_));
}

}  // namespace animax
}  // namespace lynx
