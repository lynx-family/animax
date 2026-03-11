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

#include "src/content/shape/polystar_content.h"

#include "src/content/path/path_util.h"
#include "src/layer/base_layer.h"
#include "src/model/path/shape_trim_path_model.h"
#include "src/model/shape/polystar_shape_model.h"

namespace lynx {
namespace animax {

PolystarContent::PolystarContent(BaseLayer& layer, PolystarShapeModel& model)
    : path_(std::make_unique<Path>()), layer_(layer) {
  name_ = model.name_;
  type_ = model.type_;
  hidden_ = model.hidden_;
  reversed_ = model.reversed_;

  points_animation_ = model.points_->CreateAnimation();
  position_animation_ = model.position_->CreateAnimation();
  rotation_animation_ = model.rotation_->CreateAnimation();
  outer_radius_animation_ = model.outer_radius_->CreateAnimation();
  outer_rounded_animation_ = model.outer_roundedness_->CreateAnimation();

  if (type_ == PolystarShapeType::kStar) {
    inner_radius_animation_ = model.inner_radius_->CreateAnimation();
    inner_rounded_animation_ = model.inner_roundedness_->CreateAnimation();
  }

  layer.AddAnimation(points_animation_.get());
  layer.AddAnimation(position_animation_.get());
  layer.AddAnimation(rotation_animation_.get());
  layer.AddAnimation(outer_radius_animation_.get());
  layer.AddAnimation(outer_rounded_animation_.get());

  if (type_ == PolystarShapeType::kStar) {
    layer.AddAnimation(inner_radius_animation_.get());
    layer.AddAnimation(inner_rounded_animation_.get());
  }
}

void PolystarContent::Init() {
  if (points_animation_) {
    points_animation_->AddUpdateListener(this);
  }
  if (position_animation_) {
    position_animation_->AddUpdateListener(this);
  }
  if (rotation_animation_) {
    rotation_animation_->AddUpdateListener(this);
  }
  if (outer_radius_animation_) {
    outer_radius_animation_->AddUpdateListener(this);
  }
  if (outer_rounded_animation_) {
    outer_rounded_animation_->AddUpdateListener(this);
  }

  if (type_ == PolystarShapeType::kStar) {
    if (inner_radius_animation_) {
      inner_radius_animation_->AddUpdateListener(this);
    }
    if (inner_rounded_animation_) {
      inner_rounded_animation_->AddUpdateListener(this);
    }
  }
}

Path* PolystarContent::GetPath() {
  if (is_path_valid_) {
    return path_.get();
  }

  path_->Reset();
  if (hidden_) {
    is_path_valid_ = true;
    return path_.get();
  }

  switch (type_) {
    case PolystarShapeType::kStar:
      CreateStarPath();
      break;
    case PolystarShapeType::kPolygon:
      CreatePolygonPath();
      break;
  }

  path_->Close();

  trim_paths_.Apply(*path_);

  is_path_valid_ = true;
  return path_.get();
}

void PolystarContent::CreateStarPath() {
  float points = points_animation_->GetValue().Get();
  auto current_angle = rotation_animation_ == nullptr
                           ? 0
                           : rotation_animation_->GetValue().Get();

  current_angle -= 90;
  current_angle = PathUtil::ToRadians(current_angle);

  auto angle_per_point = 2 * kPI / points;
  if (reversed_) {
    angle_per_point *= -1;
  }

  auto half_angle_per_point = angle_per_point / 2.0;
  float partial_point_amount = points - static_cast<int>(floorf(points));
  if (partial_point_amount != 0) {
    current_angle += half_angle_per_point * (1.0 - partial_point_amount);
  }

  auto outer_radius = outer_radius_animation_->GetValue().Get();
  auto inner_radius = inner_radius_animation_->GetValue().Get();

  float inner_roundedness = 0;
  if (inner_rounded_animation_) {
    inner_roundedness = inner_rounded_animation_->GetValue().Get() / 100.0;
  }

  float outer_roundedness = 0;
  if (outer_rounded_animation_) {
    outer_roundedness = outer_rounded_animation_->GetValue().Get() / 100.0;
  }

  float x, y, previous_x, previous_y, partial_point_radius = 0;
  if (partial_point_amount != 0) {
    partial_point_radius =
        inner_radius + partial_point_amount * (outer_radius - inner_radius);
    x = partial_point_radius * std::cos(current_angle);
    y = partial_point_radius * std::sin(current_angle);
    path_->MoveTo(x, y);
    current_angle += angle_per_point * partial_point_amount / 2.0;
  } else {
    x = outer_radius * std::cos(current_angle);
    y = outer_radius * std::sin(current_angle);
    path_->MoveTo(x, y);
    current_angle += half_angle_per_point;
  }

  bool long_segment = false;
  auto num_points = std::ceil(points) * 2;
  for (auto i = 0; i < num_points; i++) {
    auto radius = long_segment ? outer_radius : inner_radius;
    auto d_theta = half_angle_per_point;
    if (partial_point_radius != 0 && i == num_points - 2) {
      d_theta = angle_per_point * partial_point_amount / 2.0;
    }
    if (partial_point_radius != 0 && i == num_points - 1) {
      radius = partial_point_radius;
    }
    previous_x = x;
    previous_y = y;

    x = radius * std::cos(current_angle);
    y = radius * std::sin(current_angle);

    if (inner_roundedness == 0 && outer_roundedness == 0) {
      path_->LineTo(x, y);
    } else {
      float cp1_theta = std::atan2(previous_y, previous_x) - kPI / 2.0;
      float cp1_dx = std::cos(cp1_theta);
      float cp1_dy = std::sin(cp1_theta);

      float cp2_theta = std::atan2(y, x) - kPI / 2.0;
      float cp2_dx = std::cos(cp2_theta);
      float cp2_dy = std::sin(cp2_theta);

      float cp1_rounded = long_segment ? inner_roundedness : outer_roundedness;
      float cp2_rounded = long_segment ? outer_roundedness : inner_roundedness;
      float cp1_radius = long_segment ? inner_radius : outer_radius;
      float cp2_radius = long_segment ? outer_radius : inner_radius;

      float cp1_x = cp1_radius * cp1_rounded * kPolystarMagicNumber * cp1_dx;
      float cp1_y = cp1_radius * cp1_rounded * kPolystarMagicNumber * cp1_dy;
      float cp2_x = cp2_radius * cp2_rounded * kPolystarMagicNumber * cp2_dx;
      float cp2_y = cp2_radius * cp2_rounded * kPolystarMagicNumber * cp2_dy;

      if (partial_point_amount != 0) {
        if (i == 0) {
          cp1_x *= partial_point_amount;
          cp1_y *= partial_point_amount;
        } else if (i == num_points - 1) {
          cp2_x *= partial_point_amount;
          cp2_y *= partial_point_amount;
        }
      }

      path_->CubicTo(previous_x - cp1_x, previous_y - cp1_y, x + cp2_x,
                     y + cp2_y, x, y);
    }

    current_angle += d_theta;
    long_segment = !long_segment;
  }

  const auto& position = position_animation_->GetValue();
  path_->Offset(position.GetX(), position.GetY());
  path_->Close();
}

void PolystarContent::CreatePolygonPath() {
  auto points = std::floor(points_animation_->GetValue().Get());
  auto current_angle = rotation_animation_ == nullptr
                           ? 0
                           : rotation_animation_->GetValue().Get();

  current_angle -= 90;
  current_angle = PathUtil::ToRadians(current_angle);

  auto angle_per_point = 2.0 * kPI / points;
  auto roundedness = outer_rounded_animation_->GetValue().Get() / 100.0;
  auto radius = outer_radius_animation_->GetValue().Get();
  float x, y, previous_x, previous_y;
  x = radius * std::cos(current_angle);
  y = radius * std::sin(current_angle);
  path_->MoveTo(x, y);
  current_angle += angle_per_point;

  auto num_points = std::ceil(points);
  for (auto i = 0; i < num_points; i++) {
    previous_x = x;
    previous_y = y;

    x = radius * std::cos(current_angle);
    y = radius * std::sin(current_angle);

    if (roundedness != 0) {
      float cp1_theta = std::atan2(previous_y, previous_x) - kPI / 2.0;
      float cp1_dx = std::cos(cp1_theta);
      float cp1_dy = std::sin(cp1_theta);

      float cp2_theta = std::atan2(y, x) - kPI / 2.0;
      float cp2_dx = std::cos(cp2_theta);
      float cp2_dy = std::sin(cp2_theta);

      float cp1_x = radius * roundedness * kPolygonMagicNumber * cp1_dx;
      float cp1_y = radius * roundedness * kPolygonMagicNumber * cp1_dy;
      float cp2_x = radius * roundedness * kPolygonMagicNumber * cp2_dx;
      float cp2_y = radius * roundedness * kPolygonMagicNumber * cp2_dy;
      path_->CubicTo(previous_x - cp1_x, previous_y - cp1_y, x + cp2_x,
                     y + cp2_y, x, y);
    } else {
      path_->LineTo(x, y);
    }

    current_angle += angle_per_point;
  }

  const auto& position = position_animation_->GetValue();
  path_->Offset(position.GetX(), position.GetY());
  path_->Close();
}

void PolystarContent::SetContents(std::vector<Content*>& contents_before,
                                  std::vector<Content*>& contents_after) {
  for (auto& content : contents_before) {
    trim_paths_.AddTrimPathIfNeeds(content, this);
  }
}

void PolystarContent::OnValueChanged() { is_path_valid_ = false; }

KeyframeAnimation* PolystarContent::GetAnimationForProperty(
    LayerPropertyType type) {
  switch (type) {
    case LayerPropertyType::kPolyStarPoints: {
      return GetOrCreateAnimation<FloatKeyframeAnimation>(points_animation_,
                                                          layer_, this);
    }
    case LayerPropertyType::kPolyStarRotation: {
      return GetOrCreateAnimation<FloatKeyframeAnimation>(rotation_animation_,
                                                          layer_, this);
    }
    case LayerPropertyType::kPosition: {
      return GetOrCreateAnimation<PointKeyframeAnimation>(position_animation_,
                                                          layer_, this);
    }
    case LayerPropertyType::kPolyStarInnerRadius: {
      return GetOrCreateAnimation<FloatKeyframeAnimation>(
          inner_radius_animation_, layer_, this);
    }
    case LayerPropertyType::kPolyStarOuterRadius: {
      return GetOrCreateAnimation<FloatKeyframeAnimation>(
          outer_radius_animation_, layer_, this);
    }
    case LayerPropertyType::kPolyStarInnerRounded: {
      return GetOrCreateAnimation<FloatKeyframeAnimation>(
          inner_rounded_animation_, layer_, this);
    }
    case LayerPropertyType::kPolyStarOuterRounded: {
      return GetOrCreateAnimation<FloatKeyframeAnimation>(
          outer_rounded_animation_, layer_, this);
    }
    default: {
      return nullptr;
    }
  }
}
}  // namespace animax
}  // namespace lynx
