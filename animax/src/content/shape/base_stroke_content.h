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

#ifndef ANIMAX_SRC_CONTENT_SHAPE_BASE_STROKE_CONTENT_H_
#define ANIMAX_SRC_CONTENT_SHAPE_BASE_STROKE_CONTENT_H_

#include <vector>

#include "src/animation/keyframe_animation.h"
#include "src/content/content.h"
#include "src/content/effect/blur_element.h"
#include "src/content/effect/drop_shadow_element.h"
#include "src/content/path/trim_path_content.h"
#include "src/layer/base_layer.h"
#include "src/render/color_filter.h"
#include "src/render/path.h"
#include "src/render/path_measure.h"

namespace lynx {
namespace animax {

class ShapeStrokeModel;

class PathGroup {
 public:
  PathGroup(TrimPathContent* content) : trim_path_(content) {}
  std::vector<Content*>& GetPaths() { return paths_; }

 private:
  friend class BaseStrokeContent;

  std::vector<Content*> paths_;
  TrimPathContent* trim_path_ = nullptr;
};

class BaseStrokeContent : public AnimationListener, public Content {
 public:
  BaseStrokeContent(
      BaseLayer& layer, PaintCap cap, PaintJoin join, float miter_limit,
      std::unique_ptr<AnimatableIntegerValue>& opacity,
      std::unique_ptr<AnimatableFloatValue>& width,
      std::vector<std::shared_ptr<AnimatableFloatValue>>& dash_pattern,
      std::shared_ptr<AnimatableFloatValue>& offset);

  virtual ~BaseStrokeContent() override = default;

  void Init() override;

  void SetContents(std::vector<Content*>& contents_before,
                   std::vector<Content*>& contents_after) override;

  void OnValueChanged() override {}

  void Draw(Canvas& canvas, Matrix& parent_matrix,
            int32_t parent_alpha) override;
  void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                 bool apply_parents) override;

  bool SubDrawingType() override { return true; }

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

  std::unique_ptr<Paint> paint_;

 protected:
  BaseLayer& layer_;

 private:
  void ApplyTrimPath(Canvas& canvas, const PathGroup& path_group,
                     Matrix& parent_matrix);
  void ApplyDashPatternIfNeeded(Matrix& parent_matrix);

  std::unique_ptr<PathMeasure> pm_;
  std::unique_ptr<Path> path_;
  std::unique_ptr<Path> trim_path_;
  RectF rect_;

  std::vector<std::unique_ptr<PathGroup>> path_groups_;
  std::unique_ptr<float[]> dash_pattern_values_;
  size_t dash_value_size_ = 0;

  std::unique_ptr<FloatKeyframeAnimation> width_animation_;
  std::unique_ptr<IntegerKeyframeAnimation> opacity_animation_;
  std::vector<std::unique_ptr<FloatKeyframeAnimation>> dash_pattern_animations_;
  std::unique_ptr<FloatKeyframeAnimation> dash_pattern_offset_animation_;

  std::unique_ptr<BlurElement> blur_element_;
  std::unique_ptr<DropShadowElement> drop_shadow_element_;

  std::unique_ptr<ColorFilterKeyframeAnimation> color_filter_animation_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_SHAPE_BASE_STROKE_CONTENT_H_
