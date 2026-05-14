// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
//
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

#ifndef ANIMAX_SRC_LAYER_TEXT_LAYER_H_
#define ANIMAX_SRC_LAYER_TEXT_LAYER_H_

#include "include/base/macros.h"
#include "src/content/effect/blur_element.h"
#include "src/layer/base_layer.h"
#include "src/layer/text_layer_animations.h"
#include "src/model/value/base_value.h"
#include "src/model/value/document_data.h"

namespace lynx {
namespace animax {
class TextContent;
class TextContentDataSource;
class TextLayer : public BaseLayer {
 public:
  TextLayer(LayerModel& layer, CompositionModel& composition);
  ~TextLayer() override;

  void Init() override;
  void DrawLayer(Canvas& canvas, Matrix& matrix, int32_t alpha) override;
  void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                 bool apply_parent) override;

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

  ANIMAX_EXPORT void ApplyEffects(Paint& paint);

 private:
  TextLayerAnimations::AnimatorProperty CreateAnimator(
      const AnimatableTextProperty* text_property);
  void AddUpdateListenerToAnimatorProperty(
      TextLayerAnimations::AnimatorProperty& animator);
  TextLayerAnimations::AnimatorProperty& AnimatorPropertyForType(
      LayerPropertyType type);
  KeyframeAnimation* GetAnimationFromAnimatorProperty(
      const TextLayerAnimations::AnimatorProperty& animator,
      LayerPropertyType type);
  void CheckFontAsset();

  TextLayerAnimations animations_;
  std::unique_ptr<TextContent> text_content_;
  std::unique_ptr<TextContentDataSource> data_source_;
  std::unique_ptr<BlurElement> blur_element_;
#ifdef OS_WASM
  bool font_checked_ = false;
#endif
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXT_LAYER_H_
