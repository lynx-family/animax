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

#ifndef ANIMAX_SRC_MODEL_ANIMATABLE_ANIMATABLE_TRANSFORM_MODEL_H_
#define ANIMAX_SRC_MODEL_ANIMATABLE_ANIMATABLE_TRANSFORM_MODEL_H_

#include <memory>

#include "src/model/animatable/animatable_value.h"
#include "src/model/animatable/base_animatable_value.h"
#include "src/model/content_model.h"

namespace lynx {
namespace animax {

class TransformKeyframeAnimation;

class AnimatableTransformModel : public ContentModel {
 public:
  AnimatableTransformModel() = default;
  AnimatableTransformModel(
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
      std::unique_ptr<AnimatableOrientationValue> orientation);

  AnimatablePathValue* GetAnchorPoint() { return anchor_point_.get(); }
  AnimatablePointValue* GetPosition() { return position_.get(); }
  AnimatableScaleValue* GetScale() { return scale_.get(); }
  AnimatableFloatValue* GetRotation() { return rotation_.get(); }
  AnimatableIntegerValue* GetOpacity() { return opacity_.get(); }
  AnimatableFloatValue* GetSkew() { return skew_.get(); }
  AnimatableFloatValue* GetSkewAngle() { return skew_angle_.get(); }
  AnimatableFloatValue* GetStartOpacity() { return start_opacity_.get(); }
  AnimatableFloatValue* GetEndOpacity() { return end_opacity_.get(); }
  AnimatableFloatValue* GetXRotation() { return x_rotation_.get(); }
  AnimatableFloatValue* GetYRotation() { return y_rotation_.get(); }
  AnimatableFloatValue* GetZRotation() { return z_rotation_.get(); }
  AnimatableOrientationValue* GetOrientation() { return orientation_.get(); }

  std::unique_ptr<TransformKeyframeAnimation> CreateAnimation();

  std::unique_ptr<Content> CreateContent(CompositionModel& composition,
                                         BaseLayer& layer) override {
    // null
    return nullptr;
  }

  ModelType Type() override { return ModelType::kTransform; }

  void SetAutoOrient(bool auto_orient);

  bool IsAutoOrient();

  void SetEnable3D(bool enable_3d);
  bool GetEnable3D() const;

 private:
  std::unique_ptr<AnimatablePathValue> anchor_point_;
  std::unique_ptr<AnimatablePointValue> position_;
  std::unique_ptr<AnimatableScaleValue> scale_;
  std::unique_ptr<AnimatableFloatValue> rotation_;
  std::unique_ptr<AnimatableIntegerValue> opacity_;
  std::unique_ptr<AnimatableFloatValue> skew_;
  std::unique_ptr<AnimatableFloatValue> skew_angle_;
  std::unique_ptr<AnimatableFloatValue> start_opacity_;
  std::unique_ptr<AnimatableFloatValue> end_opacity_;
  std::unique_ptr<AnimatableFloatValue> x_rotation_;
  std::unique_ptr<AnimatableFloatValue> y_rotation_;
  std::unique_ptr<AnimatableFloatValue> z_rotation_;
  std::unique_ptr<AnimatableOrientationValue> orientation_;
  bool auto_orient_ = false;
  bool enable_3d_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_ANIMATABLE_ANIMATABLE_TRANSFORM_MODEL_H_
