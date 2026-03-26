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

#ifndef ANIMAX_SRC_CONTENT_EFFECT_BLUR_ELEMENT_H_
#define ANIMAX_SRC_CONTENT_EFFECT_BLUR_ELEMENT_H_

#include "src/animation/keyframe_animation.h"
#include "src/property/property_update_element.h"
#include "src/render/paint.h"

namespace lynx {
namespace animax {

class BaseLayer;
class FloatKeyframeAnimation;

class BlurElement : public AnimationListener, public PropertyUpdateElement {
 public:
  BlurElement(BaseLayer& layer);

  void Init();
  void Draw(Paint& paint, BaseLayer& layer, bool create_directly);

  void OnValueChanged() override {}

  float GetBlurRadius();

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

 private:
  std::unique_ptr<FloatKeyframeAnimation> blur_animation_;
  float blur_mask_filter_radius_ = 0;
  BaseLayer& layer_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_EFFECT_BLUR_ELEMENT_H_
