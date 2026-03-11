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

#ifndef ANIMAX_SRC_CONTENT_EFFECT_DROP_SHADOW_ELEMENT_H_
#define ANIMAX_SRC_CONTENT_EFFECT_DROP_SHADOW_ELEMENT_H_

#include "src/animation/drop_shadow_keyframe_animation.h"
#include "src/animation/keyframe_animation.h"
#include "src/layer/base_layer.h"
#include "src/property/property_type.h"
#include "src/render/paint.h"

namespace lynx {
namespace animax {

class DropShadowElement : public AnimationListener {
 public:
  DropShadowElement(BaseLayer& layer);

  void Init();

  void Draw(Paint& paint);

  void OnValueChanged() override {}

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type);

 private:
  std::unique_ptr<DropShadowKeyframeAnimation> drop_shadow_animation_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_EFFECT_DROP_SHADOW_ELEMENT_H_
