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

#ifndef ANIMAX_SRC_CONTENT_GRADIENT_GRADIENT_FILL_CONTENT_H_
#define ANIMAX_SRC_CONTENT_GRADIENT_GRADIENT_FILL_CONTENT_H_

#include <memory>
#include <string>

#include "src/animation/keyframe_animation.h"
#include "src/content/content.h"
#include "src/content/effect/blur_element.h"
#include "src/content/effect/drop_shadow_element.h"
#include "src/content/gradient/gradient_element.h"
#include "src/model/value/gradient_color.h"
#include "src/render/shader.h"

namespace lynx {
namespace animax {

class BaseLayer;
class GradientFillModel;

class GradientFillContent : public AnimationListener, public Content {
 public:
  GradientFillContent(BaseLayer& layer, GradientFillModel& model);
  ~GradientFillContent() override = default;

  void Init() override;

  void Draw(Canvas& canvas, Matrix& parent_matrix,
            int32_t parent_alpha) override;
  void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                 bool apply_parents) override;

  void SetContents(std::vector<Content*>& contents_before,
                   std::vector<Content*>& contents_after) override;

  void OnValueChanged() override;

  bool SubDrawingType() override { return true; }

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

 private:
  BaseLayer& layer_;

  std::unique_ptr<Path> path_;
  std::unique_ptr<Paint> paint_;
  RectF bounds_rect_;

  std::vector<Content*> paths_;

  std::unique_ptr<IntegerKeyframeAnimation> opacity_animation_;

  std::unique_ptr<GradientElement> gradient_element_;

  std::unique_ptr<BlurElement> blur_element_;
  std::unique_ptr<DropShadowElement> drop_shadow_element_;

  std::unique_ptr<ColorFilterKeyframeAnimation> color_filter_animation_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_GRADIENT_GRADIENT_FILL_CONTENT_H_
