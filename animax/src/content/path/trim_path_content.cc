// Copyright 2023 The Lynx Authors. All rights reserved.
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

#include "src/content/path/trim_path_content.h"

#include "src/layer/base_layer.h"
#include "src/model/path/shape_trim_path_model.h"

namespace lynx {
namespace animax {

class ShapeTrimPathModel;

TrimPathContent::TrimPathContent(BaseLayer& layer, ShapeTrimPathModel& model) {
  name_ = model.name_;
  hidden_ = model.hidden_;
  type_ = model.shape_trim_type_;

  start_animation_ = model.start_->CreateAnimation();
  end_animation_ = model.end_->CreateAnimation();
  offset_animation_ = model.offset_->CreateAnimation();

  layer.AddAnimation(start_animation_.get());
  layer.AddAnimation(end_animation_.get());
  layer.AddAnimation(offset_animation_.get());
}

void TrimPathContent::Init() {
  if (start_animation_) {
    start_animation_->AddUpdateListener(this);
  }
  if (end_animation_) {
    end_animation_->AddUpdateListener(this);
  }
  if (offset_animation_) {
    offset_animation_->AddUpdateListener(this);
  }
}

FloatKeyframeAnimation& TrimPathContent::GetStart() const {
  return *start_animation_;
}

FloatKeyframeAnimation& TrimPathContent::GetEnd() const {
  return *end_animation_;
}

FloatKeyframeAnimation& TrimPathContent::GetOffset() const {
  return *offset_animation_;
}

void CompoundTrimPathContent::AddTrimPathIfNeeds(Content* content,
                                                 AnimationListener* listener) {
  if (content->MainType() == ContentType::kTrimPath) {
    auto trim_content = static_cast<TrimPathContent*>(content);
    if (trim_content->GetType() == ShapeTrimType::kSimultaneously) {
      AddTrimPath(trim_content);
      trim_content->AddListener(listener);
    }
  }
}

}  // namespace animax
}  // namespace lynx
