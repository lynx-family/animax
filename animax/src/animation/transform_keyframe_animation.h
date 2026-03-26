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

#ifndef ANIMAX_SRC_ANIMATION_TRANSFORM_KEYFRAME_ANIMATION_H_
#define ANIMAX_SRC_ANIMATION_TRANSFORM_KEYFRAME_ANIMATION_H_

#include <memory>

#include "src/animation/base_keyframe_animation.h"
#include "src/animation/keyframe_animation.h"
#include "src/property/property_type.h"
#include "src/property/property_update_element.h"
#include "src/render/matrix.h"

namespace lynx {
namespace animax {

class AnimatableTransformModel;
class BaseLayer;

class TransformKeyframeAnimation : public PropertyUpdateElement {
 public:
  TransformKeyframeAnimation(AnimatableTransformModel& model);

  void AddAnimationToLayer(BaseLayer& layer);
  void AddListener(AnimationListener* listener);

  Matrix& GetMatrix();
  Matrix& GetMatrixForRepeater(float amount);

  IntegerKeyframeAnimation* GetOpacity() { return opacity_.get(); }
  FloatKeyframeAnimation* GetStartOpacity() { return start_opacity_.get(); }
  FloatKeyframeAnimation* GetEndOpacity() { return end_opacity_.get(); }
  PointKeyframeAnimation* GetAnchorPoint() { return anchor_point_.get(); }
  PointKeyframeAnimation* GetPosition() { return position_.get(); }
  FloatKeyframeAnimation* GetXRotation() { return x_rotation_.get(); }
  FloatKeyframeAnimation* GetYRotation() { return y_rotation_.get(); }
  FloatKeyframeAnimation* GetZRotation() { return z_rotation_.get(); }
  OrientationKeyframeAnimation* GetOrientation() { return orientation_.get(); }

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

  void SetProgress(float progress);

 private:
  friend class BaseLayer;

  void ClearSkewValues();

  std::unique_ptr<Matrix> matrix_;
  std::unique_ptr<Matrix> skew_matrix1_;
  std::unique_ptr<Matrix> skew_matrix2_;
  std::unique_ptr<Matrix> skew_matrix3_;
  std::unique_ptr<float[]> skew_values_;

  std::unique_ptr<PointKeyframeAnimation> anchor_point_;
  std::unique_ptr<PointKeyframeAnimation> position_;
  std::unique_ptr<ScaleKeyframeAnimation> scale_;
  std::unique_ptr<FloatKeyframeAnimation> rotation_;
  std::unique_ptr<IntegerKeyframeAnimation> opacity_;

  std::unique_ptr<FloatKeyframeAnimation> skew_;
  std::unique_ptr<FloatKeyframeAnimation> skew_angle_;

  std::unique_ptr<FloatKeyframeAnimation> start_opacity_;
  std::unique_ptr<FloatKeyframeAnimation> end_opacity_;

  std::unique_ptr<FloatKeyframeAnimation> x_rotation_;
  std::unique_ptr<FloatKeyframeAnimation> y_rotation_;
  std::unique_ptr<FloatKeyframeAnimation> z_rotation_;
  std::unique_ptr<OrientationKeyframeAnimation> orientation_;

  AnimationHost* host_ = nullptr;

  bool auto_orient_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_ANIMATION_TRANSFORM_KEYFRAME_ANIMATION_H_
