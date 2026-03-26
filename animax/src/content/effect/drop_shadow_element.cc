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

#include "src/content/effect/drop_shadow_element.h"

namespace lynx {
namespace animax {

DropShadowElement::DropShadowElement(BaseLayer& layer)
    : drop_shadow_animation_(std::make_unique<DropShadowKeyframeAnimation>(
          this, layer, *layer.GetDropEffect())) {}

void DropShadowElement::Init() {
  if (drop_shadow_animation_) {
    drop_shadow_animation_->Init();
  }
}

void DropShadowElement::Draw(Paint& paint) {
  if (drop_shadow_animation_) {
    drop_shadow_animation_->ApplyTo(paint);
  }
}

KeyframeAnimation* DropShadowElement::GetAnimationForProperty(
    LayerPropertyType type) {
  if (drop_shadow_animation_) {
    return drop_shadow_animation_->GetAnimationForProperty(type);
  }
  return nullptr;
}

}  // namespace animax
}  // namespace lynx
