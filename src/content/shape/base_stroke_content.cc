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

#include "src/content/shape/base_stroke_content.h"

#include <algorithm>

#include "src/content/path/path_util.h"
#include "src/model/path/shape_trim_path_model.h"
#include "src/model/shape/shape_stroke_model.h"

namespace lynx {
namespace animax {

BaseStrokeContent::BaseStrokeContent(
    BaseLayer& layer, PaintCap cap, PaintJoin join, float miter_limit,
    std::unique_ptr<AnimatableIntegerValue>& opacity,
    std::unique_ptr<AnimatableFloatValue>& width,
    std::vector<std::shared_ptr<AnimatableFloatValue>>& dash_pattern,
    std::shared_ptr<AnimatableFloatValue>& offset)
    : paint_(std::make_unique<Paint>()),
      layer_(layer),
      pm_(std::make_unique<PathMeasure>()),
      path_(std::make_unique<Path>()),
      trim_path_(std::make_unique<Path>()) {
  paint_->SetAntiAlias(true);
  paint_->SetStyle(PaintStyle::kStroke);
  paint_->SetStrokeCap(cap);
  paint_->SetStrokeJoin(join);
  paint_->SetStrokeMiter(miter_limit);

  opacity_animation_ = opacity->CreateAnimation();
  width_animation_ = width->CreateAnimation();

  if (offset == nullptr) {
    dash_pattern_offset_animation_ = nullptr;
  } else {
    dash_pattern_offset_animation_ = offset->CreateAnimation();
  }

  dash_value_size_ = dash_pattern.size();
  dash_pattern_values_ = std::make_unique<float[]>(dash_value_size_);
  for (auto& dash : dash_pattern) {
    dash_pattern_animations_.push_back(dash->CreateAnimation());
  }

  layer_.AddAnimation(opacity_animation_.get());
  layer_.AddAnimation(width_animation_.get());
  for (auto& animation : dash_pattern_animations_) {
    layer_.AddAnimation(animation.get());
  }

  if (dash_pattern_offset_animation_) {
    layer_.AddAnimation(dash_pattern_offset_animation_.get());
  }

  if (layer.GetBlurEffect()) {
    blur_element_ = std::unique_ptr<BlurElement>(new BlurElement(layer));
  }

  if (layer_.GetDropEffect()) {
    drop_shadow_element_ =
        std::unique_ptr<DropShadowElement>(new DropShadowElement(layer));
  }
}

void BaseStrokeContent::Init() {
  if (opacity_animation_) {
    opacity_animation_->AddUpdateListener(this);
  }
  if (width_animation_) {
    width_animation_->AddUpdateListener(this);
  }

  for (auto& dash : dash_pattern_animations_) {
    dash->AddUpdateListener(this);
  }

  if (dash_pattern_offset_animation_) {
    dash_pattern_offset_animation_->AddUpdateListener(this);
  }

  if (blur_element_) {
    blur_element_->Init();
  }

  if (drop_shadow_element_) {
    drop_shadow_element_->Init();
  }
}

void BaseStrokeContent::SetContents(std::vector<Content*>& contents_before,
                                    std::vector<Content*>& contents_after) {
  TrimPathContent* trim_path_before = nullptr;
  for (auto it = contents_before.rbegin(); it != contents_before.rend(); it++) {
    if ((*it)->MainType() == ContentType::kTrimPath) {
      auto trim_content = static_cast<TrimPathContent*>(*it);
      if (trim_content->type_ == ShapeTrimType::kIndividually) {
        trim_path_before = trim_content;
      }
    }
  }

  if (trim_path_before) {
    trim_path_before->AddListener(this);
  }

  std::unique_ptr<PathGroup> current_path_group;
  for (auto it = contents_after.rbegin(); it != contents_after.rend(); it++) {
    auto& content = *it;
    if (content->MainType() == ContentType::kTrimPath) {
      auto trim_content = static_cast<TrimPathContent*>(*it);
      if (trim_content->type_ == ShapeTrimType::kIndividually) {
        if (current_path_group) {
          path_groups_.push_back(std::move(current_path_group));
        }
        current_path_group =
            std::unique_ptr<PathGroup>(new PathGroup(trim_content));
        trim_content->AddListener(this);
      }
    }

    if (content->SubPathType()) {
      if (current_path_group == nullptr) {
        current_path_group =
            std::unique_ptr<PathGroup>(new PathGroup(trim_path_before));
      }
      current_path_group->GetPaths().push_back(content);
    }
  }

  if (current_path_group) {
    path_groups_.push_back(std::move(current_path_group));
  }
}

void BaseStrokeContent::Draw(Canvas& canvas, Matrix& parent_matrix,
                             int32_t parent_alpha) {
  // TODO(aiyongbiao): has zero axis p1

  auto alpha =
      (parent_alpha / 255.0 * opacity_animation_->GetValue().Get() / 100.0) *
      255.0;
  paint_->SetAlpha(std::clamp(alpha, 0.0, 255.0));

  paint_->SetStrokeWidth(width_animation_->GetValue().Get() *
                         parent_matrix.GetScale());
  if (paint_->GetStrokeWidth() <= 0) {
    return;
  }

  ApplyDashPatternIfNeeded(parent_matrix);

  if (color_filter_animation_) {
    paint_->SetColorFilter(color_filter_animation_->GetValue());
  }

  if (blur_element_) {
    blur_element_->Draw(*paint_, layer_, false);
  }

  if (drop_shadow_element_) {
    drop_shadow_element_->Draw(*paint_);
  }

  for (auto& path_group : path_groups_) {
    if (path_group->trim_path_) {
      ApplyTrimPath(canvas, *path_group, parent_matrix);
    } else {
      path_->Reset();
      for (auto it = path_group->paths_.rbegin();
           it != path_group->paths_.rend(); it++) {
        path_->AddPath((*it)->GetPath(), parent_matrix);
      }
      canvas.DrawPath(*path_, *paint_);
    }
  }
}

void BaseStrokeContent::GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                                  bool apply_parents) {
  path_->Reset();
  for (auto& path_group : path_groups_) {
    for (auto& path : path_group->paths_) {
      path_->AddPath(path->GetPath(), parent_matrix);
    }
  }
  path_->ComputeBounds(rect_, false);

  auto width = width_animation_->GetValue().Get();
  rect_.Set(rect_.GetLeft() - width / 2.0, rect_.GetTop() - width / 2.0,
            rect_.GetRight() + width / 2.0, rect_.GetBottom() + width / 2.0);
  out_bounds.Set(rect_);
  out_bounds.Set(out_bounds.GetLeft() - 1, out_bounds.GetTop() - 1,
                 out_bounds.GetRight() + 1, out_bounds.GetBottom() + 1);
}

void BaseStrokeContent::ApplyTrimPath(Canvas& canvas,
                                      const PathGroup& path_group,
                                      Matrix& parent_matrix) {
  if (path_group.trim_path_ == nullptr) {
    return;
  }

  path_->Reset();
  for (auto it = path_group.paths_.rbegin(); it != path_group.paths_.rend();
       it++) {
    path_->AddPath((*it)->GetPath(), parent_matrix);
  }

  auto trim_path = path_group.trim_path_;
  auto anim_start = trim_path->start_animation_->GetValue().Get() / 100.0;
  auto anim_end = trim_path->end_animation_->GetValue().Get() / 100.0;
  auto anim_offset = trim_path->offset_animation_->GetValue().Get() / 360.0;

  if (anim_start < 0.01 && anim_end > 0.99) {
    canvas.DrawPath(*path_, *paint_);
    return;
  }

  pm_->SetPath(*path_, false);
  auto total_length = pm_->GetLength();
  while (pm_->NextContour()) {
    total_length += pm_->GetLength();
  }

  auto offset_length = total_length * anim_offset;
  auto start_length = total_length * anim_start + offset_length;
  auto end_length = std::min(total_length * anim_end + offset_length,
                             start_length + total_length - 1.0);

  auto current_length = 0;
  auto paths = path_group.paths_;
  for (auto it = paths.rbegin(); it != paths.rend(); it++) {
    trim_path_->Set((*it)->GetPath());
    trim_path_->Transform(parent_matrix);
    pm_->SetPath(*trim_path_, false);

    auto length = pm_->GetLength();
    if (end_length > total_length &&
        end_length - total_length < current_length + length &&
        current_length < end_length - total_length) {
      float start_value = 0;
      if (start_length > total_length) {
        start_value = (start_length - total_length) / length;
      }
      float end_value = std::min((end_length - total_length) / length, 1.0);
      PathUtil::ApplyTrimPathIfNeeds(*trim_path_, start_value, end_value, 0);
      canvas.DrawPath(*trim_path_, *paint_);
    } else {
      if (current_length + length < start_length ||
          current_length > end_length) {
        // do noting
      } else if (current_length + length <= end_length &&
                 start_length < current_length) {
        canvas.DrawPath(*trim_path_, *paint_);
      } else {
        float start_value;
        if (start_length < current_length) {
          start_value = 0;
        } else {
          start_value = (start_length - current_length) / length;
        }
        float end_value;
        if (end_length > current_length + length) {
          end_value = 1.0;
        } else {
          end_value = (end_length - current_length) / length;
        }
        PathUtil::ApplyTrimPathIfNeeds(*trim_path_, start_value, end_value, 0);
        canvas.DrawPath(*trim_path_, *paint_);
      }
    }
    current_length += length;
  }
}

void BaseStrokeContent::ApplyDashPatternIfNeeded(Matrix& parent_matrix) {
  if (dash_pattern_animations_.empty() || dash_pattern_values_ == nullptr) {
    return;
  }
  auto scale = parent_matrix.GetScale();
  for (auto i = 0; i < dash_value_size_; i++) {
    dash_pattern_values_[i] = dash_pattern_animations_[i]->GetValue().Get();
    if (i % 2 == 0) {
      if (dash_pattern_values_[i] < 1.f) {
        dash_pattern_values_[i] = 1.f;
      }
    } else {
      if (dash_pattern_values_[i] < 0.1f) {
        dash_pattern_values_[i] = 0.1f;
      }
    }
    dash_pattern_values_[i] *= scale;
  }
  auto offset = dash_pattern_offset_animation_ == nullptr
                    ? 0.f
                    : dash_pattern_offset_animation_->GetValue().Get() * scale;
  paint_->SetDashPathEffect(*DashPathEffect::Make(dash_pattern_values_.get(),
                                                  dash_value_size_, offset));
}

KeyframeAnimation* BaseStrokeContent::GetAnimationForProperty(
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
    case LayerPropertyType::kStrokeWidth: {
      return GetOrCreateAnimation<FloatKeyframeAnimation>(width_animation_,
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
