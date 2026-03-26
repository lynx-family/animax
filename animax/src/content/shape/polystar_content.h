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

#ifndef ANIMAX_SRC_CONTENT_SHAPE_POLYSTAR_CONTENT_H_
#define ANIMAX_SRC_CONTENT_SHAPE_POLYSTAR_CONTENT_H_

#include "src/animation/keyframe_animation.h"
#include "src/content/content.h"
#include "src/content/path/trim_path_content.h"
#include "src/layer/base_layer.h"

namespace lynx {
namespace animax {

static constexpr float kPolystarMagicNumber = .47829;
static constexpr float kPolygonMagicNumber = .25;

class PolystarShapeModel;
enum class PolystarShapeType : uint8_t;

class PolystarContent : public AnimationListener, public Content {
 public:
  PolystarContent(BaseLayer& layer, PolystarShapeModel& model);

  void Init() override;

  Path* GetPath() override;

  void SetContents(std::vector<Content*>& contents_before,
                   std::vector<Content*>& contents_after) override;

  void OnValueChanged() override;

  void CreateStarPath();
  void CreatePolygonPath();

  bool SubPathType() override { return true; }

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

 private:
  std::unique_ptr<Path> path_;
  PolystarShapeType type_;
  bool reversed_ = false;

  std::unique_ptr<FloatKeyframeAnimation> points_animation_;
  std::unique_ptr<PointKeyframeAnimation> position_animation_;
  std::unique_ptr<FloatKeyframeAnimation> rotation_animation_;
  std::unique_ptr<FloatKeyframeAnimation> inner_radius_animation_;
  std::unique_ptr<FloatKeyframeAnimation> outer_radius_animation_;
  std::unique_ptr<FloatKeyframeAnimation> inner_rounded_animation_;
  std::unique_ptr<FloatKeyframeAnimation> outer_rounded_animation_;

  CompoundTrimPathContent trim_paths_;
  bool is_path_valid_ = false;

  BaseLayer& layer_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_SHAPE_POLYSTAR_CONTENT_H_
