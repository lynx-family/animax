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
#include "src/animation/text_keyframe_animation.h"

namespace lynx {
namespace animax {

DocumentData& TextKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                              float progress) {
  DocumentData* value = nullptr;
  if (progress != 1.0 || keyframe.IsEndValueEmpty()) {
    value = keyframe.GetStartValue<DocumentData>();
  } else {
    value = keyframe.GetEndValue<DocumentData>();
  }

  if (HasValueCallback()) {
    auto callback_value =
        GetValueFromCallback(keyframe.GetStartValue<DocumentData>(),
                             keyframe.GetStartValue<DocumentData>(),
                             keyframe.GetEndValue<DocumentData>(), progress,
                             keyframe.GetStartFrame(), keyframe.GetEndFrame());
    if (callback_value) {
      if (auto* typed_value =
              reinterpret_cast<DocumentData*>(callback_value.get())) {
        if (value) {
          intermediate_.reset(
              static_cast<DocumentData*>(value->Copy().release()));
          intermediate_->SetText(typed_value->GetText());
        }
      }
    }
    return *intermediate_;
  } else {
    return value ? *value : *intermediate_;
  }
}

DocumentData& TextKeyframeAnimation::GetValue() {
  return static_cast<DocumentData&>(KeyframeAnimation::GetValue());
}

std::unique_ptr<TextKeyframeAnimation> TextKeyframeAnimation::MakeDefault() {
  auto model_list = std::shared_ptr<KeyframeModelList>(new KeyframeModelList());
  model_list->push_back(std::unique_ptr<KeyframeModel>(
      new KeyframeModel(ValueFactory::Make<DocumentData>())));
  return std::unique_ptr<TextKeyframeAnimation>(
      new TextKeyframeAnimation(std::move(model_list)));
}

}  // namespace animax
}  // namespace lynx
