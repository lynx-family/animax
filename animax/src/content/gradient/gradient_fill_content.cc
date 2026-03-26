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

#include "src/content/gradient/gradient_fill_content.h"

#include <algorithm>

#include "src/layer/base_layer.h"
#include "src/model/gradient/gradient_fill_model.h"
#include "src/render/mask_filter.h"

namespace lynx {
namespace animax {

GradientFillContent::GradientFillContent(BaseLayer& layer,
                                         GradientFillModel& model)
    : layer_(layer),
      path_(std::make_unique<Path>()),
      paint_(std::make_unique<Paint>()) {
  name_ = model.name_;
  hidden_ = model.hidden_;

  if (model.gradient_color_ && model.start_point_ && model.end_point_) {
    gradient_element_ = std::make_unique<GradientElement>(
        layer, *model.gradient_color_, *model.start_point_, *model.end_point_,
        model.gradient_type_, model.highlight_length_.get(),
        model.highlight_angle_.get(), true);
  }

  paint_->SetAntiAlias(true);
  path_->SetFillType(model.fill_type_);

  opacity_animation_ = model.opacity_->CreateAnimation();
  layer_.AddAnimation(opacity_animation_.get());

  if (layer_.GetBlurEffect()) {
    blur_element_ = std::unique_ptr<BlurElement>(new BlurElement(layer));
  }

  if (layer_.GetDropEffect()) {
    drop_shadow_element_ =
        std::unique_ptr<DropShadowElement>(new DropShadowElement(layer));
  }
}

void GradientFillContent::Init() {
  if (gradient_element_) {
    gradient_element_->Init();
  }

  if (opacity_animation_) {
    opacity_animation_->AddUpdateListener(this);
  }

  if (blur_element_) {
    blur_element_->Init();
  }

  if (drop_shadow_element_) {
    drop_shadow_element_->Init();
  }
}

void GradientFillContent::SetContents(std::vector<Content*>& contents_before,
                                      std::vector<Content*>& contents_after) {
  for (auto& content : contents_after) {
    if (content->SubPathType()) {
      paths_.push_back(content);
    }
  }
}

void GradientFillContent::Draw(Canvas& canvas, Matrix& parent_matrix,
                               int32_t parent_alpha) {
  if (hidden_) {
    return;
  }

  path_->Reset();
  for (auto& path : paths_) {
    path_->AddPath(path->GetPath(), parent_matrix);
  }

  path_->ComputeBounds(bounds_rect_, false);

  if (color_filter_animation_) {
    paint_->SetColorFilter(color_filter_animation_->GetValue());
  }

  if (gradient_element_) {
    gradient_element_->Draw(*paint_, parent_matrix);
  }

  if (blur_element_) {
    blur_element_->Draw(*paint_, layer_, true);
  }

  if (drop_shadow_element_) {
    drop_shadow_element_->Draw(*paint_);
  }

  auto alpha =
      (parent_alpha / 255.0 * opacity_animation_->GetValue().Get() / 100.0) *
      255.0;
  paint_->SetAlpha(std::clamp(alpha, 0.0, 255.0));

  canvas.DrawPath(*path_, *paint_);
}

void GradientFillContent::GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                                    bool apply_parent) {
  path_->Reset();
  for (auto& path : paths_) {
    path_->AddPath(path->GetPath(), parent_matrix);
  }

  path_->ComputeBounds(out_bounds, false);
  out_bounds.Set(out_bounds.GetLeft() - 1, out_bounds.GetTop() - 1,
                 out_bounds.GetRight() + 1, out_bounds.GetBottom() + 1);

  if (blur_element_) {
    float radius = blur_element_->GetBlurRadius();
    if (radius != 0.f) {
      float l = out_bounds.GetLeft();
      float t = out_bounds.GetTop();
      float r = out_bounds.GetRight();
      float b = out_bounds.GetBottom();

      out_bounds.Set(l - radius, t - radius, r + radius, b + radius);
    }
  }
}

void GradientFillContent::OnValueChanged() {}

KeyframeAnimation* GradientFillContent::GetAnimationForProperty(
    LayerPropertyType type) {
  // Blur element
  if (blur_element_) {
    auto* animation = blur_element_->GetAnimationForProperty(type);
    if (animation) {
      return animation;
    }
  }

  // Drop shadow element
  if (drop_shadow_element_) {
    auto* animation = drop_shadow_element_->GetAnimationForProperty(type);
    if (animation) {
      return animation;
    }
  }

  // Other animations
  switch (type) {
    case LayerPropertyType::kOpacity: {
      return GetOrCreateAnimation<IntegerKeyframeAnimation>(opacity_animation_,
                                                            layer_, this);
    }
    case LayerPropertyType::kColorFilter: {
      return GetOrCreateAnimation<ColorFilterKeyframeAnimation>(
          color_filter_animation_, layer_, this);
    }
    default: {
      return nullptr;
    }
  }
}

}  // namespace animax
}  // namespace lynx
