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

#ifndef ANIMAX_SRC_MODEL_ANIMATABLE_ANIMATABLE_TEXT_PROPERTIES_H_
#define ANIMAX_SRC_MODEL_ANIMATABLE_ANIMATABLE_TEXT_PROPERTIES_H_

#include "src/animation/text_keyframe_animation.h"
#include "src/model/animatable/animatable_text_range_selector.h"
#include "src/model/animatable/animatable_value.h"
#include "src/model/animatable/base_animatable_value.h"
#include "src/model/value/document_data.h"

namespace lynx {
namespace animax {

class AnimatableFloatValue;
class AnimatableColorValue;
class AnimatableTextRangeSelector;

class AnimatableTextProperty {
 public:
  AnimatableTextProperty() = default;
  AnimatableTextProperty(std::unique_ptr<AnimatableColorValue> color,
                         std::unique_ptr<AnimatableColorValue> stroke,
                         std::unique_ptr<AnimatableFloatValue> stroke_width,
                         std::unique_ptr<AnimatableFloatValue> tracking,
                         std::unique_ptr<AnimatableFloatValue> skew)
      : color_(std::move(color)),
        stroke_(std::move(stroke)),
        stroke_width_(std::move(stroke_width)),
        tracking_(std::move(tracking)),
        skew_(std::move(skew)) {}

  AnimatableColorValue* GetColor() const {
    return color_ ? color_.get() : nullptr;
  }
  AnimatableColorValue* GetStroke() const {
    return stroke_ ? stroke_.get() : nullptr;
  }
  AnimatableFloatValue* GetStrokeWidth() const {
    return stroke_width_ ? stroke_width_.get() : nullptr;
  }
  AnimatableFloatValue* GetTracking() const {
    return tracking_ ? tracking_.get() : nullptr;
  }
  AnimatableFloatValue* GetSkew() const {
    return skew_ ? skew_.get() : nullptr;
  }
  AnimatableTextRangeSelector* GetRangeSelector() const {
    return range_selector_ ? range_selector_.get() : nullptr;
  }
  void SetRangeSelector(
      std::unique_ptr<AnimatableTextRangeSelector> range_selector) {
    range_selector_ = std::move(range_selector);
  }

 private:
  std::unique_ptr<AnimatableTextRangeSelector> range_selector_;
  std::unique_ptr<AnimatableColorValue> color_;
  std::unique_ptr<AnimatableColorValue> stroke_;
  std::unique_ptr<AnimatableFloatValue> stroke_width_;
  std::unique_ptr<AnimatableFloatValue> tracking_;
  std::unique_ptr<AnimatableFloatValue> skew_;
};

using AnimatableTextPropertyList =
    std::vector<std::unique_ptr<AnimatableTextProperty>>;

class AnimatableTextFrame : public AnimatableValue {
 public:
  std::unique_ptr<TextKeyframeAnimation> CreateAnimation() {
    return std::unique_ptr<TextKeyframeAnimation>(
        new TextKeyframeAnimation(frames_));
  }
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_ANIMATABLE_ANIMATABLE_TEXT_PROPERTIES_H_
