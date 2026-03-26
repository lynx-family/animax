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

#include "src/content/shape/rectangle_content.h"

#include <algorithm>

#include "src/content/shape/rounded_corners_content.h"
#include "src/model/path/shape_trim_path_model.h"
#include "src/model/shape/rectangle_shape_model.h"

namespace lynx {
namespace animax {

RectangleContent::RectangleContent(BaseLayer& layer, RectangleShapeModel& model)
    : path_(std::make_unique<Path>()), layer_(layer) {
  name_ = model.name_;
  hidden_ = model.hidden_;

  position_animation_ = model.position_->CreateAnimation();
  size_animation_ = model.size_->CreateAnimation();
  corner_radius_animation_ = model.roundedness_->CreateAnimation();

  layer.AddAnimation(position_animation_.get());
  layer.AddAnimation(size_animation_.get());
  layer.AddAnimation(corner_radius_animation_.get());
}

void RectangleContent::Init() {
  if (position_animation_) {
    position_animation_->AddUpdateListener(this);
  }
  if (size_animation_) {
    size_animation_->AddUpdateListener(this);
  }
  if (corner_radius_animation_) {
    corner_radius_animation_->AddUpdateListener(this);
  }
}

void RectangleContent::OnValueChanged() { is_path_valid_ = false; }

Path* RectangleContent::GetPath() {
  if (is_path_valid_ && rounded_corners_animation_ == nullptr) {
    return path_.get();
  }

  path_->Reset();
  if (hidden_) {
    is_path_valid_ = true;
    return path_.get();
  }

  const auto& size = size_animation_->GetValue();
  auto half_width = size.GetX() / 2;
  auto half_height = size.GetY() / 2;
  float radius = corner_radius_animation_ == nullptr
                     ? 0
                     : corner_radius_animation_->GetValue().Get();
  if (radius == 0 && rounded_corners_animation_) {
    radius = std::min(rounded_corners_animation_->GetValue().Get(),
                      std::min(half_width, half_height));
  }
  auto max_radius = std::min(half_width, half_height);
  if (radius > max_radius) {
    radius = max_radius;
  }

  const auto& position = position_animation_->GetValue();
  path_->MoveTo(position.GetX() + half_width,
                position.GetY() - half_height + radius);

  path_->LineTo(position.GetX() + half_width,
                position.GetY() + half_height - radius);
  if (radius > 0) {
    rect_.Set(position.GetX() + half_width - 2 * radius,
              position.GetY() + half_height - 2 * radius,
              position.GetX() + half_width, position.GetY() + half_height);
    path_->ArcTo(rect_, 0, 90, false);
  }

  path_->LineTo(position.GetX() - half_width + radius,
                position.GetY() + half_height);
  if (radius > 0) {
    rect_.Set(position.GetX() - half_width,
              position.GetY() + half_height - 2 * radius,
              position.GetX() - half_width + 2 * radius,
              position.GetY() + half_height);
    path_->ArcTo(rect_, 90, 90, false);
  }

  path_->LineTo(position.GetX() - half_width,
                position.GetY() - half_height + radius);
  if (radius > 0) {
    rect_.Set(position.GetX() - half_width, position.GetY() - half_height,
              position.GetX() - half_width + 2 * radius,
              position.GetY() - half_height + 2 * radius);
    path_->ArcTo(rect_, 180, 90, false);
  }

  path_->LineTo(position.GetX() + half_width - radius,
                position.GetY() - half_height);
  if (radius > 0) {
    rect_.Set(position.GetX() + half_width - 2 * radius,
              position.GetY() - half_height, position.GetX() + half_width,
              position.GetY() - half_height + 2 * radius);
    path_->ArcTo(rect_, 270, 90, false);
  }

  path_->Close();

  trim_paths_.Apply(*path_);

  is_path_valid_ = true;
  return path_.get();
}

void RectangleContent::SetContents(std::vector<Content*>& contents_before,
                                   std::vector<Content*>& contents_after) {
  for (auto& content : contents_before) {
    trim_paths_.AddTrimPathIfNeeds(content, this);

    if (content->MainType() == ContentType::kRoundCorner) {
      auto rounded_content = static_cast<RoundedCornersContent*>(content);
      rounded_corners_animation_ = rounded_content->GetRoundedCorners();
    }
  }
}

KeyframeAnimation* RectangleContent::GetAnimationForProperty(
    LayerPropertyType type) {
  switch (type) {
    case LayerPropertyType::kRectangleSize: {
      return GetOrCreateAnimation<PointKeyframeAnimation>(size_animation_,
                                                          layer_, this);
    }
    case LayerPropertyType::kPosition: {
      return GetOrCreateAnimation<PointKeyframeAnimation>(position_animation_,
                                                          layer_, this);
    }
    case LayerPropertyType::kCornerRadius: {
      return GetOrCreateAnimation<FloatKeyframeAnimation>(
          corner_radius_animation_, layer_, this);
    }
    default: {
      return nullptr;
    }
  }
}

}  // namespace animax
}  // namespace lynx
