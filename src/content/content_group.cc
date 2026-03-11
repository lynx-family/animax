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

#include "src/content/content_group.h"

#include "src/base/log/log.h"
#include "src/model/animatable/animatable_transform_model.h"
#include "src/model/shape/shape_group_model.h"

namespace lynx {
namespace animax {

ContentGroup::ContentGroup(BaseLayer& layer, ShapeGroupModel& shape_group,
                           CompositionModel& composition)
    : ContentGroup(layer, shape_group.name_, shape_group.hidden_,
                   FindTransform(shape_group.GetItems())) {
  ContentsFromModels(composition, layer, shape_group.GetItems(), contents_);
}

ContentGroup::ContentGroup(BaseLayer& layer, const std::string& name,
                           bool hidden, AnimatableTransformModel* transform)
    : matrix_(std::make_unique<Matrix>()), path_(std::make_unique<Path>()) {
  name_ = name;
  hidden_ = hidden;

  if (transform) {
    transform_animation_ = transform->CreateAnimation();
    transform_animation_->AddAnimationToLayer(layer);
  }
}

void ContentGroup::Init() {
  if (transform_animation_) {
    transform_animation_->AddListener(this);
  }

  std::vector<Content*> greedy_contents;
  for (auto it = contents_.rbegin(); it != contents_.rend(); it++) {
    if ((*it).get()->SubGreedyType()) {
      greedy_contents.push_back((*it).get());
    }
  }

  for (auto it = greedy_contents.rbegin(); it != greedy_contents.rend(); it++) {
    auto greedy_content = *it;
    greedy_content->AbsorbContent(contents_);
  }
}

Path* ContentGroup::GetPath() {
  matrix_->Reset();
  if (transform_animation_) {
    matrix_->Set(transform_animation_->GetMatrix());
  }

  path_->Reset();
  if (hidden_) {
    return path_.get();
  }

  for (auto it = contents_.rbegin(); it != contents_.rend(); it++) {
    auto path_content = (*it).get();
    if (path_content->SubPathType()) {
      path_->AddPath(path_content->GetPath(), *matrix_);
    }
  }
  return path_.get();
}

void ContentGroup::SetContents(std::vector<Content*>& contents_before,
                               std::vector<Content*>& contents_after) {
  std::vector<Content*> combined_contents;
  combined_contents.insert(combined_contents.end(), contents_before.begin(),
                           contents_before.end());

  auto index = contents_.size() - 1;
  auto begin_it = contents_.begin();
  for (auto it = contents_.rbegin(); it != contents_.rend(); it++) {
    auto content = contents_[index].get();
    auto sub_contents = std::vector<Content*>();
    for (auto i = 0; i < index; i++) {
      sub_contents.push_back((begin_it + i)->get());
    }
    content->SetContents(combined_contents, sub_contents);
    combined_contents.push_back(content);
    index--;
  }
}

void ContentGroup::Draw(Canvas& canvas, Matrix& parent_matrix,
                        int32_t parent_alpha) {
  if (hidden_) {
    return;
  }

  matrix_->Set(parent_matrix);

  auto layer_alpha = 0;
  if (transform_animation_) {
    matrix_->PreConcat(transform_animation_->GetMatrix());
    auto opacity = transform_animation_->GetOpacity();
    auto opacity_value = 100;
    if (opacity) {
      opacity_value = opacity->GetValue().Get();
    }
    layer_alpha = (opacity_value / 100.0 * parent_alpha / 255.0) * 255.0;
  } else {
    layer_alpha = parent_alpha;
  }

  // TODO(aiyongbiao): offscreen p1

  auto child_alpha = layer_alpha;  // TODO(aiyongbiao): offscreen alpha
  for (auto it = contents_.rbegin(); it != contents_.rend(); it++) {
    auto drawing_content = (*it).get();
    if (drawing_content->SubDrawingType()) {
      drawing_content->Draw(canvas, *matrix_, child_alpha);
    }
  }

  // TODO(aiyongbiao): offscreen render p1
}

void ContentGroup::ContentsFromModels(
    CompositionModel& composition, BaseLayer& layer,
    std::vector<ContentModel*>& content_models,
    std::vector<std::unique_ptr<Content>>& contents) {
  for (auto* content_model : content_models) {
    auto content = content_model->CreateContent(composition, layer);
    if (content) {
      content->Init();
      contents.push_back(std::move(content));
    }
  }
}

void ContentGroup::GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                             bool apply_parent) {
  matrix_->Set(parent_matrix);
  if (transform_animation_) {
    matrix_->PreConcat(transform_animation_->GetMatrix());
  }
  rect_.Set(0, 0, 0, 0);
  for (auto it = contents_.rbegin(); it != contents_.rend(); it++) {
    auto drawing_content = (*it).get();
    if (drawing_content->SubDrawingType()) {
      drawing_content->GetBounds(rect_, *matrix_, apply_parent);
      out_bounds.Union(rect_);
    }
  }
}

void ContentGroup::OnValueChanged() {
  // nothing
}

AnimatableTransformModel* ContentGroup::FindTransform(
    const std::vector<ContentModel*>& content_models) {
  for (auto& content_model : content_models) {
    if (content_model->Type() == ModelType::kTransform) {
      return static_cast<AnimatableTransformModel*>(content_model);
    }
  }
  return nullptr;
}

void ContentGroup::GetPathList(std::vector<Content*>& path_contents) {
  for (auto& content : contents_) {
    if (content->SubPathType()) {
      path_contents.push_back(content.get());
    }
  }
}

Matrix& ContentGroup::GetTransformationMatrix() {
  if (transform_animation_) {
    return transform_animation_->GetMatrix();
  }
  matrix_->Reset();
  return *matrix_;
}

void ContentGroup::ResolveKeyPath(const AnimaXKeyPath& path, int32_t depth,
                                  ResolvedKeyPathElements& match_elements,
                                  const AnimaXKeyPath& current_partial_path) {
  auto result = KeyPathResolveUtil::ResolveElement(
      *this, path, depth, match_elements, current_partial_path);
  if (result.should_propagate_to_child_) {
    for (auto& content : contents_) {
      content->ResolveKeyPath(path, result.child_depth_, match_elements,
                              result.updated_key_path_);
    }
  }
}

KeyframeAnimation* ContentGroup::GetAnimationForProperty(
    LayerPropertyType type) {
  if (transform_animation_) {
    auto* animation = transform_animation_->GetAnimationForProperty(type);
    if (animation) {
      return animation;
    }
  }
  return Content::GetAnimationForProperty(type);
}

}  // namespace animax
}  // namespace lynx
