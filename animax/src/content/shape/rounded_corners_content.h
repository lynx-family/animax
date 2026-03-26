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

#ifndef ANIMAX_SRC_CONTENT_SHAPE_ROUNDED_CORNERS_CONTENT_H_
#define ANIMAX_SRC_CONTENT_SHAPE_ROUNDED_CORNERS_CONTENT_H_

#include "src/animation/keyframe_animation.h"
#include "src/content/shape/shape_modifier_content.h"
#include "src/layer/base_layer.h"

namespace lynx {
namespace animax {

/**
 * Copied from:
 * https://github.com/airbnb/lottie-web/blob/bb71072a26e03f1ca993da60915860f39aae890b/player/js/utils/common.js#L47
 */
static constexpr float kRoundedCornerMagicNumber = 0.5519;

class RoundedCornersModel;

class RoundedCornersContent : public ShapeModifierContent,
                              public AnimationListener {
 public:
  RoundedCornersContent(BaseLayer& layer, RoundedCornersModel& model);

  void Init() override;

  ContentType MainType() override { return ContentType::kRoundCorner; }

  void SetContents(std::vector<Content*>& contents_before,
                   std::vector<Content*>& contents_after) override;

  ShapeData ModifyShape(ShapeData& shape_data) override;

  void OnValueChanged() override;

  FloatKeyframeAnimation* GetRoundedCorners() { return rounded_corners_.get(); }

 private:
  ShapeData& GetShapeData(ShapeData& starting_shape_data);

  std::unique_ptr<FloatKeyframeAnimation> rounded_corners_;
  ShapeData shape_data_ = {};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_CONTENT_SHAPE_ROUNDED_CORNERS_CONTENT_H_
