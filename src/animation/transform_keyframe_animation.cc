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

#include "src/animation/transform_keyframe_animation.h"

#include <memory>

#include "src/content/path/path_util.h"
#include "src/layer/base_layer.h"
#include "src/model/animatable/animatable_transform_model.h"
#include "src/model/value/base_value.h"
#include "src/render/quaternion.h"

namespace lynx {
namespace animax {

TransformKeyframeAnimation::TransformKeyframeAnimation(
    AnimatableTransformModel& model)
    : matrix_(std::make_unique<Matrix>()) {
  const bool enable_3d = model.GetEnable3D();
  auto* anchor = model.GetAnchorPoint();
  if (anchor) {
    anchor_point_ = anchor->CreateAnimation(enable_3d);
  }

  auto* position = model.GetPosition();
  if (position) {
    position_ = position->CreateAnimation(enable_3d);
  }

  auto* scale = model.GetScale();
  if (scale) {
    scale_ = scale->CreateAnimation();
  }

  auto* rotation = model.GetRotation();
  if (rotation) {
    rotation_ = rotation->CreateAnimation();
  }

  auto* skew = model.GetSkew();
  if (skew) {
    skew_matrix1_ = std::make_unique<Matrix>();
    skew_matrix2_ = std::make_unique<Matrix>();
    skew_matrix3_ = std::make_unique<Matrix>();
    skew_ = skew->CreateAnimation();
  }

  auto_orient_ = model.IsAutoOrient();

  if (skew_) {
    skew_values_ = std::make_unique<float[]>(9);
  }

  auto* skew_angle = model.GetSkewAngle();
  if (skew_angle) {
    skew_angle_ = skew_angle->CreateAnimation();
  }

  auto* opacity = model.GetOpacity();
  if (opacity) {
    opacity_ = opacity->CreateAnimation();
  }

  auto* start_opacity = model.GetStartOpacity();
  if (start_opacity) {
    start_opacity_ = start_opacity->CreateAnimation();
  }

  auto* end_opacity = model.GetEndOpacity();
  if (end_opacity) {
    end_opacity_ = end_opacity->CreateAnimation();
  }

  auto* x_rotation = model.GetXRotation();
  if (x_rotation) {
    x_rotation_ = x_rotation->CreateAnimation();
  }

  auto* y_rotation = model.GetYRotation();
  if (y_rotation) {
    y_rotation_ = y_rotation->CreateAnimation();
  }

  auto* z_rotation = model.GetZRotation();
  if (z_rotation) {
    z_rotation_ = z_rotation->CreateAnimation();
  }

  auto* orientation = model.GetOrientation();
  if (orientation) {
    orientation_ = orientation->CreateAnimation();
  }
}

void TransformKeyframeAnimation::AddAnimationToLayer(BaseLayer& layer) {
  host_ = &layer;

  layer.AddAnimation(opacity_.get());
  layer.AddAnimation(start_opacity_.get());
  layer.AddAnimation(end_opacity_.get());

  layer.AddAnimation(anchor_point_.get());
  layer.AddAnimation(position_.get());
  layer.AddAnimation(scale_.get());
  layer.AddAnimation(rotation_.get());

  layer.AddAnimation(skew_.get());
  layer.AddAnimation(skew_angle_.get());

  layer.AddAnimation(x_rotation_.get());
  layer.AddAnimation(y_rotation_.get());
  layer.AddAnimation(z_rotation_.get());
  layer.AddAnimation(orientation_.get());
}

void TransformKeyframeAnimation::AddListener(AnimationListener* listener) {
  if (opacity_) {
    opacity_->AddUpdateListener(listener);
  }

  if (start_opacity_) {
    start_opacity_->AddUpdateListener(listener);
  }

  if (end_opacity_) {
    end_opacity_->AddUpdateListener(listener);
  }

  if (anchor_point_) {
    anchor_point_->AddUpdateListener(listener);
  }

  if (position_) {
    position_->AddUpdateListener(listener);
  }

  if (scale_) {
    scale_->AddUpdateListener(listener);
  }

  if (rotation_) {
    rotation_->AddUpdateListener(listener);
  }

  if (skew_) {
    skew_->AddUpdateListener(listener);
  }

  if (skew_angle_) {
    skew_angle_->AddUpdateListener(listener);
  }

  if (x_rotation_) {
    x_rotation_->AddUpdateListener(listener);
  }

  if (y_rotation_) {
    y_rotation_->AddUpdateListener(listener);
  }

  if (z_rotation_) {
    z_rotation_->AddUpdateListener(listener);
  }

  if (orientation_) {
    orientation_->AddUpdateListener(listener);
  }
}

Matrix& TransformKeyframeAnimation::GetMatrixForRepeater(float amount) {
  auto* position = position_ == nullptr ? nullptr : &position_->GetValue();
  auto* scale = scale_ == nullptr ? nullptr : &scale_->GetValue();

  matrix_->Reset();
  if (position) {
    matrix_->PreTranslate(position->GetX() * amount, position->GetY() * amount,
                          position->GetZ() * amount);
  }
  if (scale) {
    matrix_->PreScale(std::pow(scale->GetX(), amount),
                      std::pow(scale->GetY(), amount),
                      std::pow(scale->GetZ(), amount));
  }
  if (rotation_) {
    auto rotation = rotation_->GetValue().Get();
    auto* anchor_point =
        anchor_point_ == nullptr ? nullptr : &anchor_point_->GetValue();
    matrix_->PreRotate(rotation * amount,
                       anchor_point == nullptr ? 0 : anchor_point->GetX(),
                       anchor_point == nullptr ? 0 : anchor_point->GetY());
  }
  return *matrix_;
}

Matrix& TransformKeyframeAnimation::GetMatrix() {
  matrix_->Reset();
  if (position_) {
    const auto& position_value = position_->GetValue();
    if (!position_value.IsEmpty() &&
        (position_value.GetX() != 0 || position_value.GetY() != 0 ||
         position_value.GetZ() != 0)) {
      matrix_->PreTranslate(position_value.GetX(), position_value.GetY(),
                            position_value.GetZ());
    }
  }

  if (auto_orient_) {
    if (position_) {
      auto current_progress = position_->GetProgress();
      const auto& start_position = position_->GetValue();
      // Store the start X and Y values because the PointF will be overwritten
      // by the next GetValue call.
      auto start_x = start_position.GetX();
      auto start_y = start_position.GetY();
      // 1. Find the next position value.
      // 2. Create a vector from the current position to the next position.
      // 3. Find the angle of that vector to the X axis (0 degrees).
      position_->SetProgress(current_progress + 0.0001f);
      const auto& next_position = position_->GetValue();
      position_->SetProgress(current_progress);
      auto rotation_value = PathUtil::ToDegrees(std::atan2(
          next_position.GetY() - start_y, next_position.GetX() - start_x));
      matrix_->PreRotate(rotation_value);
    }
  } else if (rotation_) {
    // 2D rotation
    const auto& rotation_value = rotation_->GetValue();

    if (!rotation_value.IsEmpty() && rotation_value.Get() != 0) {
      matrix_->PreRotate(rotation_value.Get());
    }
  } else {
    // 3D rotation
    if (orientation_) {
      auto orientation_matrix = orientation_->GetValue().ToMatrix();
      if (orientation_matrix) {
        matrix_->PreConcat(*orientation_matrix);
      }
    }
    if (x_rotation_) {
      auto x_degree =
          x_rotation_ ? std::fmod(x_rotation_->GetValue().Get(), 360.f) : 0.f;
      matrix_->PreRotateXYZ(x_degree, 0, 0);
    }
    if (y_rotation_) {
      auto y_degree =
          y_rotation_ ? std::fmod(y_rotation_->GetValue().Get(), 360.f) : 0.f;
      matrix_->PreRotateXYZ(0, y_degree, 0);
    }
    if (z_rotation_) {
      auto z_degree =
          z_rotation_ ? std::fmod(z_rotation_->GetValue().Get(), 360.f) : 0.f;
      matrix_->PreRotateXYZ(0, 0, z_degree);
    }
  }

  if (skew_) {
    auto cos = skew_angle_ == nullptr
                   ? 0.0
                   : std::cos(PathUtil::ToRadians(
                         -skew_angle_->GetValue().Get() + 90.0));
    auto sin = skew_angle_ == nullptr
                   ? 1.0
                   : std::sin(PathUtil::ToRadians(
                         -skew_angle_->GetValue().Get() + 90.0));
    auto tan = std::tan(PathUtil::ToRadians(skew_->GetValue().Get()));

    ClearSkewValues();
    skew_values_[0] = cos;
    skew_values_[1] = sin;
    skew_values_[3] = -sin;
    skew_values_[4] = cos;
    skew_values_[8] = 1.0;
    skew_matrix1_->SetValues(skew_values_.get());
    ClearSkewValues();
    skew_values_[0] = 1.0;
    skew_values_[3] = tan;
    skew_values_[4] = 1.0;
    skew_values_[8] = 1.0;
    skew_matrix2_->SetValues(skew_values_.get());
    ClearSkewValues();
    skew_values_[0] = cos;
    skew_values_[1] = -sin;
    skew_values_[3] = sin;
    skew_values_[4] = cos;
    skew_values_[8] = 1.0;
    skew_matrix3_->SetValues(skew_values_.get());
    skew_matrix2_->PreConcat(*skew_matrix1_);
    skew_matrix3_->PreConcat(*skew_matrix2_);

    matrix_->PreConcat(*skew_matrix3_);
  }

  if (scale_) {
    const auto& scale_value = scale_->GetValue();
    if (!scale_value.IsEmpty() &&
        (scale_value.GetX() != 1 || scale_value.GetY() != 1 ||
         scale_value.GetZ() != 1)) {
      matrix_->PreScale(scale_value.GetX(), scale_value.GetY(),
                        scale_value.GetZ());
    }
  }

  if (anchor_point_) {
    const auto& anchor_value = anchor_point_->GetValue();
    if (!anchor_value.IsEmpty() &&
        (anchor_value.GetX() != 0 || anchor_value.GetY() != 0 ||
         anchor_value.GetZ() != 0)) {
      matrix_->PreTranslate(-anchor_value.GetX(), -anchor_value.GetY(),
                            -anchor_value.GetZ());
    }
  }

  return *matrix_;
}

void TransformKeyframeAnimation::SetProgress(float progress) {
  if (opacity_) {
    opacity_->SetProgress(progress);
  }

  if (start_opacity_) {
    start_opacity_->SetProgress(progress);
  }

  if (end_opacity_) {
    end_opacity_->SetProgress(progress);
  }

  if (anchor_point_) {
    anchor_point_->SetProgress(progress);
  }

  if (position_) {
    position_->SetProgress(progress);
  }

  if (scale_) {
    scale_->SetProgress(progress);
  }

  if (rotation_) {
    rotation_->SetProgress(progress);
  }

  if (skew_) {
    skew_->SetProgress(progress);
  }

  if (skew_angle_) {
    skew_angle_->SetProgress(progress);
  }

  if (x_rotation_) {
    x_rotation_->SetProgress(progress);
  }

  if (y_rotation_) {
    y_rotation_->SetProgress(progress);
  }

  if (z_rotation_) {
    z_rotation_->SetProgress(progress);
  }

  if (orientation_) {
    orientation_->SetProgress(progress);
  }
}

void TransformKeyframeAnimation::ClearSkewValues() {
  for (auto i = 0; i < 9; i++) {
    skew_values_[i] = 0.0;
  }
}

KeyframeAnimation* TransformKeyframeAnimation::GetAnimationForProperty(
    LayerPropertyType type) {
  if (host_ == nullptr) {
    return nullptr;
  }

  auto& host = *host_;
  switch (type) {
    case LayerPropertyType::kTransformPosition:
      return GetOrCreateAnimation<PointKeyframeAnimation>(position_, host,
                                                          nullptr);
    case LayerPropertyType::kTransformScale:
      return GetOrCreateAnimation<ScaleKeyframeAnimation>(scale_, host,
                                                          nullptr);
    case LayerPropertyType::kTransformAnchor:
      return GetOrCreateAnimation<PointKeyframeAnimation>(anchor_point_, host,
                                                          nullptr);
    case LayerPropertyType::kTransformOpacity:
      return GetOrCreateAnimation<IntegerKeyframeAnimation>(opacity_, host,
                                                            nullptr);
    case LayerPropertyType::kTransformStartOpacity:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(start_opacity_, host,
                                                          nullptr);
    case LayerPropertyType::kTransformEndOpacity:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(end_opacity_, host,
                                                          nullptr);
    case LayerPropertyType::kTransformSkew:
      if (skew_ == nullptr) {
        skew_matrix1_ = std::make_unique<Matrix>();
        skew_matrix2_ = std::make_unique<Matrix>();
        skew_matrix3_ = std::make_unique<Matrix>();
        skew_values_ = std::make_unique<float[]>(9);
      }
      return GetOrCreateAnimation<FloatKeyframeAnimation>(skew_, host, nullptr);
    case LayerPropertyType::kTransformSkewAngle:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(skew_angle_, host,
                                                          nullptr);
    case LayerPropertyType::kTransformRotation:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(rotation_, host,
                                                          nullptr);
    case LayerPropertyType::kTransformRotationX:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(x_rotation_, host,
                                                          nullptr);
    case LayerPropertyType::kTransformRotationY:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(y_rotation_, host,
                                                          nullptr);
    case LayerPropertyType::kTransformRotationZ:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(z_rotation_, host,
                                                          nullptr);
      // TODO(aiyongbiao.rick): support orientation property update.
    default:
      return nullptr;
  }
}

}  // namespace animax
}  // namespace lynx
