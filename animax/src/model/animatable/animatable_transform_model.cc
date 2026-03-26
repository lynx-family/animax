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

#include "src/model/animatable/animatable_transform_model.h"

#include "src/animation/transform_keyframe_animation.h"

namespace lynx {
namespace animax {

AnimatableTransformModel::AnimatableTransformModel(
    std::unique_ptr<AnimatablePathValue> anchor_point,
    std::unique_ptr<AnimatablePointValue> position,
    std::unique_ptr<AnimatableScaleValue> scale,
    std::unique_ptr<AnimatableFloatValue> rotation,
    std::unique_ptr<AnimatableIntegerValue> opacity,
    std::unique_ptr<AnimatableFloatValue> skew,
    std::unique_ptr<AnimatableFloatValue> skewAngle,
    std::unique_ptr<AnimatableFloatValue> start_opacity,
    std::unique_ptr<AnimatableFloatValue> end_opacity,
    std::unique_ptr<AnimatableFloatValue> x_rotation,
    std::unique_ptr<AnimatableFloatValue> y_rotation,
    std::unique_ptr<AnimatableFloatValue> z_rotation,
    std::unique_ptr<AnimatableOrientationValue> orientation)
    : anchor_point_(std::move(anchor_point)),
      position_(std::move(position)),
      scale_(std::move(scale)),
      rotation_(std::move(rotation)),
      opacity_(std::move(opacity)),
      skew_(std::move(skew)),
      skew_angle_(std::move(skewAngle)),
      start_opacity_(std::move(start_opacity)),
      end_opacity_(std::move(end_opacity)),
      x_rotation_(std::move(x_rotation)),
      y_rotation_(std::move(y_rotation)),
      z_rotation_(std::move(z_rotation)),
      orientation_(std::move(orientation)) {}

void AnimatableTransformModel::SetAutoOrient(bool auto_orient) {
  auto_orient_ = auto_orient;
}

bool AnimatableTransformModel::IsAutoOrient() { return auto_orient_; }

std::unique_ptr<TransformKeyframeAnimation>
AnimatableTransformModel::CreateAnimation() {
  return std::unique_ptr<TransformKeyframeAnimation>(
      new TransformKeyframeAnimation(*this));
}

void AnimatableTransformModel::SetEnable3D(bool enable_3d) {
  enable_3d_ = enable_3d;
}

bool AnimatableTransformModel::GetEnable3D() const { return enable_3d_; }

}  // namespace animax
}  // namespace lynx
