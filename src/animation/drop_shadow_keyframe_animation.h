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

#ifndef ANIMAX_SRC_ANIMATION_DROP_SHADOW_KEYFRAME_ANIMATION_H_
#define ANIMAX_SRC_ANIMATION_DROP_SHADOW_KEYFRAME_ANIMATION_H_

#include <memory>
#include <vector>

#include "src/animation/base_keyframe_animation.h"
#include "src/animation/keyframe_animation.h"
#include "src/content/path/path_util.h"
#include "src/model/effect/drop_shadow_effect_model.h"
#include "src/model/value/base_value.h"
#include "src/property/property_update_element.h"
#include "src/render/paint.h"

namespace lynx {
namespace animax {

class BaseLayer;

class DropShadowKeyframeAnimation : public AnimationListener,
                                    public PropertyUpdateElement {
 public:
  DropShadowKeyframeAnimation(AnimationListener* listener, BaseLayer& layer,
                              DropShadowEffectModel& model);

  void Init();
  void OnValueChanged() override;

  void ApplyTo(Paint& paint);

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

 private:
  AnimationListener* listener_ = nullptr;
  std::unique_ptr<ColorKeyframeAnimation> color_;
  std::unique_ptr<FloatKeyframeAnimation> opacity_;
  std::unique_ptr<FloatKeyframeAnimation> direction_;
  std::unique_ptr<FloatKeyframeAnimation> distance_;
  std::unique_ptr<FloatKeyframeAnimation> radius_;

  bool is_dirty_ = true;
  BaseLayer& layer_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_ANIMATION_DROP_SHADOW_KEYFRAME_ANIMATION_H_
