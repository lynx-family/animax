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

#include "src/content/shape/repeater_content.h"

#include "src/base/util/misc_util.h"
#include "src/model/shape/repeater_model.h"
#include "src/property/key_path_resolve_util.h"

namespace lynx {
namespace animax {

RepeaterContent::RepeaterContent(BaseLayer& layer, RepeaterModel& model)
    : matrix_(std::make_unique<Matrix>()),
      path_(std::make_unique<Path>()),
      layer_(layer),
      copies_(model.copies_->CreateAnimation()) {
  name_ = model.name_;
  hidden_ = model.hidden_;

  layer.AddAnimation(copies_.get());

  offset_ = model.offset_->CreateAnimation();
  layer.AddAnimation(offset_.get());

  transform_ = model.transform_->CreateAnimation();
  transform_->AddAnimationToLayer(layer);
}

void RepeaterContent::Init() {
  if (copies_) {
    copies_->AddUpdateListener(this);
  }
  if (offset_) {
    offset_->AddUpdateListener(this);
  }
  if (transform_) {
    transform_->AddListener(this);
  }
}

void RepeaterContent::Draw(Canvas& canvas, Matrix& parent_matrix,
                           int32_t parent_alpha) {
  if (content_group_ == nullptr) {
    return;
  }
  auto copies = static_cast<int>(copies_->GetValue().Get());
  auto offset = offset_->GetValue().Get();

  float start_opacity = transform_->GetStartOpacity()->GetValue().Get() / 100.0;
  float end_opacity = transform_->GetEndOpacity()->GetValue().Get() / 100.0;

  for (auto i = copies; i > 0; i--) {
    matrix_->Set(parent_matrix);
    matrix_->PreConcat(transform_->GetMatrixForRepeater(i - 1 + offset));
    auto new_alpha = parent_alpha * Lerp(start_opacity, end_opacity,
                                         static_cast<float>(i - 1 / copies));
    content_group_->Draw(canvas, *matrix_, new_alpha);
  }
}

void RepeaterContent::GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                                bool apply_parents) {
  if (content_group_ == nullptr) {
    return;
  }
  content_group_->GetBounds(out_bounds, parent_matrix, apply_parents);
}

void RepeaterContent::SetContents(std::vector<Content*>& contents_before,
                                  std::vector<Content*>& contents_after) {
  if (content_group_ == nullptr) {
    return;
  }
  content_group_->SetContents(contents_before, contents_after);
}

Path* RepeaterContent::GetPath() {
  path_->Reset();
  if (content_group_ == nullptr) {
    return path_.get();
  }
  auto content_path = content_group_->GetPath();
  auto copies = copies_->GetValue().Get();
  auto offset = offset_->GetValue().Get();

  for (auto i = 0; i < copies; i++) {
    auto index = copies - i - 1;
    matrix_->Set(transform_->GetMatrixForRepeater(index + offset));
    path_->AddPath(content_path, *matrix_);
  }
  return path_.get();
}

void RepeaterContent::OnValueChanged() {}

void RepeaterContent::AbsorbContent(
    std::vector<std::unique_ptr<Content>>& contents) {
  if (content_group_) {
    return;
  }

  std::string name = "Repeater";
  content_group_ = std::unique_ptr<ContentGroup>(
      new ContentGroup(layer_, name, hidden_, nullptr));
  content_group_->Init();
  auto& new_contents = content_group_->GetContents();

  bool find_content = false;
  for (auto it = contents.rbegin(); it != contents.rend(); it++) {
    auto& content = *it;
    if (!find_content && content.get() == this) {
      find_content = true;
      continue;
    }

    if (find_content) {
      new_contents.push_back(std::move(content));
      contents.erase(std::next(it).base());
    }
  }

  std::reverse(new_contents.begin(), new_contents.end());
}

void RepeaterContent::ResolveKeyPath(
    const AnimaXKeyPath& path, int32_t depth,
    ResolvedKeyPathElements& match_elements,
    const AnimaXKeyPath& current_partial_path) {
  KeyPathResolveUtil::FullyResolveElement(*this, path, depth, match_elements,
                                          current_partial_path);
  auto& contents = content_group_->GetContents();
  for (auto& content : contents) {
    content->ResolveKeyPath(path, depth, match_elements, current_partial_path);
  }
}

KeyframeAnimation* RepeaterContent::GetAnimationForProperty(
    LayerPropertyType type) {
  if (transform_) {
    auto* animation = transform_->GetAnimationForProperty(type);
    if (animation) {
      return animation;
    }
  }

  switch (type) {
    case LayerPropertyType::kRepeaterCopies: {
      return GetOrCreateAnimation<FloatKeyframeAnimation>(copies_, layer_,
                                                          this);
    }
    case LayerPropertyType::kRepeaterOffset: {
      return GetOrCreateAnimation<FloatKeyframeAnimation>(offset_, layer_,
                                                          this);
    }
    default: {
      return nullptr;
    }
  }
}
}  // namespace animax
}  // namespace lynx
