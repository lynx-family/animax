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

#ifndef ANIMAX_SRC_MODEL_EFFECT_DROP_SHADOW_EFFECT_MODEL_H_
#define ANIMAX_SRC_MODEL_EFFECT_DROP_SHADOW_EFFECT_MODEL_H_

#include <memory>

#include "src/model/animatable/base_animatable_value.h"

namespace lynx {
namespace animax {

class DropShadowKeyframeAnimation;

class DropShadowEffectModel {
 public:
  DropShadowEffectModel(std::unique_ptr<AnimatableColorValue> color,
                        std::unique_ptr<AnimatableFloatValue> opacity,
                        std::unique_ptr<AnimatableFloatValue> direction,
                        std::unique_ptr<AnimatableFloatValue> distance,
                        std::unique_ptr<AnimatableFloatValue> radius)
      : color_(std::move(color)),
        opacity_(std::move(opacity)),
        direction_(std::move(direction)),
        distance_(std::move(distance)),
        radius_(std::move(radius)) {}

 private:
  friend class DropShadowKeyframeAnimation;

  std::unique_ptr<AnimatableColorValue> color_;
  std::unique_ptr<AnimatableFloatValue> opacity_;
  std::unique_ptr<AnimatableFloatValue> direction_;
  std::unique_ptr<AnimatableFloatValue> distance_;
  std::unique_ptr<AnimatableFloatValue> radius_;

  bool IsValid() {
    return color_ && opacity_ && direction_ && distance_ && radius_;
  }
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_EFFECT_DROP_SHADOW_EFFECT_MODEL_H_
