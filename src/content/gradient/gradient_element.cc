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

#include "src/content/gradient/gradient_element.h"

#include <algorithm>
#include <cmath>

#include "src/layer/base_layer.h"
#include "src/model/gradient/gradient_fill_model.h"

namespace lynx {
namespace animax {

GradientElement::GradientElement(
    BaseLayer& layer, AnimatableGradientValue& gradient_color,
    AnimatablePointValue& start_point, AnimatablePointValue& end_point,
    GradientType type, AnimatableFloatValue* highlight_length,
    AnimatableFloatValue* highlight_angle, bool keep_min_radius) {
  type_ = type;
  cache_steps_ = layer.GetComposition().GetDuration() / 32.0;
  keep_min_radius_ = keep_min_radius;

  color_animation_ = gradient_color.CreateAnimation();
  layer.AddAnimation(color_animation_.get());

  start_point_animation_ = start_point.CreateAnimation();
  layer.AddAnimation(start_point_animation_.get());

  end_point_animation_ = end_point.CreateAnimation();
  layer.AddAnimation(end_point_animation_.get());

  if (highlight_length) {
    highlight_length_animation_ = highlight_length->CreateAnimation();
    layer.AddAnimation(highlight_length_animation_.get());
  }
  if (highlight_angle) {
    highlight_angle_animation_ = highlight_angle->CreateAnimation();
    layer.AddAnimation(highlight_angle_animation_.get());
  }
}

void GradientElement::Init() {
  if (color_animation_) {
    color_animation_->AddUpdateListener(this);
  }
  if (start_point_animation_) {
    start_point_animation_->AddUpdateListener(this);
  }
  if (end_point_animation_) {
    end_point_animation_->AddUpdateListener(this);
  }
  if (highlight_length_animation_) {
    highlight_length_animation_->AddUpdateListener(this);
  }
  if (highlight_angle_animation_) {
    highlight_angle_animation_->AddUpdateListener(this);
  }
}

void GradientElement::Draw(Paint& paint, Matrix& parent_matrix) {
  std::unique_ptr<Shader> shader;
  if (type_ == GradientType::kLinear) {
    shader = GetLinearGradient(parent_matrix);
  } else {
    shader = GetRadialGradient(parent_matrix);
  }
  paint.SetShader(shader.get());
}

std::unique_ptr<Shader> GradientElement::GetLinearGradient(Matrix& matrix) {
  //    auto gradient_hash = GetGradientHash();
  //    auto gradient = linear_gradient_cache_[gradient_hash];
  //    if (gradient) {
  //        return gradient;
  //    }
  const auto& start_point = start_point_animation_->GetValue();
  const auto& end_point = end_point_animation_->GetValue();
  const auto& gradient_color = color_animation_->GetValue();

  // TODO(aiyongbiao): dynamic colors
  auto colors = gradient_color.GetColors();
  auto positions = gradient_color.GetPositions();
  auto gradient =
      Shader::MakeLinear(start_point, end_point, gradient_color.GetSize(),
                         colors, positions, ShaderTileMode::kClamp, matrix);

  //    linear_gradient_cache_[gradient_hash] = gradient; // TODO(aiyongbiao):
  //    support setLocalMatrix
  return gradient;
}

std::unique_ptr<Shader> GradientElement::GetRadialGradient(Matrix& matrix) {
  //    auto gradient_hash = GetGradientHash();
  //    auto gradient = radial_gradient_cache_[gradient_hash];
  //    if (gradient) {
  //        return gradient;
  //    }
  const auto& start_point = start_point_animation_->GetValue();
  const auto& end_point = end_point_animation_->GetValue();
  const auto& gradient_color = color_animation_->GetValue();

  // TODO(aiyongbiao): dynamic colors
  auto colors = gradient_color.GetColors();
  auto positions = gradient_color.GetPositions();
  auto x0 = start_point.GetX();
  auto y0 = start_point.GetY();
  auto x1 = end_point.GetX();
  auto y1 = end_point.GetY();
  auto r = std::hypot(x1 - x0, y1 - y0);
  if (keep_min_radius_ && r <= 0) {
    r = 0.001;
  }

  std::unique_ptr<Shader> gradient;
  constexpr float max_highlight_length =
      99.9f;  // change to 100 after fixing skity bug
  float highlight_length =
      highlight_length_animation_
          ? std::clamp(highlight_length_animation_->GetValue().Get(),
                       -max_highlight_length, max_highlight_length)
          : 0.f;
  if (highlight_length == 0.f || r <= 1e-3) {
    gradient =
        Shader::MakeRadial(start_point, r, gradient_color.GetSize(), colors,
                           positions, ShaderTileMode::kClamp, matrix);
  } else {
    float highlight_angle = (highlight_angle_animation_
                                 ? highlight_angle_animation_->GetValue().Get()
                                 : 0.f);
    constexpr float kPI = 3.14159265358979323846f;
    float length = highlight_length * r / 100.f;
    float angle =
        std::atan2(y1 - y0, x1 - x0) + (highlight_angle * kPI / 180.0f);
    auto x2 = x0 + std::cos(angle) * length;
    auto y2 = y0 + std::sin(angle) * length;
    gradient = Shader::MakeTwoPointConical(
        PointF(x2, y2, 0), 0, start_point, r, gradient_color.GetSize(), colors,
        positions, ShaderTileMode::kClamp, matrix);
  }

  //    radial_gradient_cache_[gradient_hash] = gradient;
  return gradient;
}

int32_t GradientElement::GetGradientHash() {
  auto start_point_progress =
      std::round(start_point_animation_->GetProgress() * cache_steps_);
  auto end_point_progress =
      std::round(end_point_animation_->GetProgress() * cache_steps_);
  auto color_progress =
      std::round(color_animation_->GetProgress() * cache_steps_);
  auto hash = 17;
  if (start_point_progress != 0) {
    hash = hash * 31 * start_point_progress;
  }
  if (end_point_progress != 0) {
    hash = hash * 31 * end_point_progress;
  }
  if (color_progress != 0) {
    hash = hash * 31 * color_progress;
  }
  return hash;
}

int32_t* GradientElement::ApplyDynamicColorIfNeeds(int32_t* colors) {
  return nullptr;  // TODO(aiyongbiao): p0
}

}  // namespace animax
}  // namespace lynx
