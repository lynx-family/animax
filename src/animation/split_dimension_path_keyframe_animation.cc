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

#include "src/animation/split_dimension_path_keyframe_animation.h"

namespace lynx {
namespace animax {

SplitDimensionPathKeyframeAnimation::SplitDimensionPathKeyframeAnimation(
    std::unique_ptr<FloatKeyframeAnimation> x_animation,
    std::unique_ptr<FloatKeyframeAnimation> y_animation,
    std::shared_ptr<KeyframeModelList> frames)
    : PointKeyframeAnimation(std::move(frames)),
      x_animation_(std::move(x_animation)),
      y_animation_(std::move(y_animation)) {
  Init();
}

void SplitDimensionPathKeyframeAnimation::Init() { SetProgress(GetProgress()); }

void SplitDimensionPathKeyframeAnimation::SetProgress(float progress) {
  x_animation_->SetProgress(progress);
  y_animation_->SetProgress(progress);
  point_.Set(x_animation_->GetValue().Get(), y_animation_->GetValue().Get(), 0);
  for (auto& listener : listeners_) {
    listener->OnValueChanged();
  }
}

PointF& SplitDimensionPathKeyframeAnimation::GetValue(KeyframeModel& keyframe,
                                                      float progress) {
  return GetValue();
}

PointF& SplitDimensionPathKeyframeAnimation::GetValue() {
  intermediate_.Set(point_.GetX(), 0, 0);
  intermediate_.Set(intermediate_.GetX(), point_.GetY(), 0);
  return intermediate_;
}

}  // namespace animax
}  // namespace lynx
