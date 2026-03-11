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

#ifndef ANIMAX_SRC_MODEL_SHAPE_POLYSTAR_SHAPE_MODEL_H_
#define ANIMAX_SRC_MODEL_SHAPE_POLYSTAR_SHAPE_MODEL_H_

#include <string>

#include "src/content/shape/polystar_content.h"
#include "src/model/animatable/base_animatable_value.h"
#include "src/model/content_model.h"

namespace lynx {
namespace animax {

enum class PolystarShapeType : uint8_t { kStar = 1, kPolygon };

class PolystarShapeModel : public ContentModel {
 public:
  PolystarShapeModel(std::string name, int32_t type_int,
                     std::unique_ptr<AnimatableFloatValue> points,
                     std::unique_ptr<AnimatablePointValue> position,
                     std::unique_ptr<AnimatableFloatValue> rotation,
                     std::unique_ptr<AnimatableFloatValue> outer_radius,
                     std::unique_ptr<AnimatableFloatValue> outer_roundedness,
                     std::unique_ptr<AnimatableFloatValue> inner_radius,
                     std::unique_ptr<AnimatableFloatValue> inner_roundedness,
                     bool hidden, bool reversed)
      : points_(std::move(points)),
        position_(std::move(position)),
        rotation_(std::move(rotation)),
        outer_radius_(std::move(outer_radius)),
        outer_roundedness_(std::move(outer_roundedness)),
        inner_radius_(std::move(inner_radius)),
        inner_roundedness_(std::move(inner_roundedness)),
        reversed_(reversed) {
    name_ = std::move(name);
    hidden_ = hidden;
    switch (type_int) {
      case 1:
        type_ = PolystarShapeType::kStar;
        break;
      case 2:
        type_ = PolystarShapeType::kPolygon;
        break;
    }
  }

  std::unique_ptr<Content> CreateContent(CompositionModel& composition,
                                         BaseLayer& layer) override;

 private:
  friend class PolystarContent;

  PolystarShapeType type_;
  std::unique_ptr<AnimatableFloatValue> points_;
  std::unique_ptr<AnimatablePointValue> position_;
  std::unique_ptr<AnimatableFloatValue> rotation_;
  std::unique_ptr<AnimatableFloatValue> outer_radius_;
  std::unique_ptr<AnimatableFloatValue> outer_roundedness_;
  std::unique_ptr<AnimatableFloatValue> inner_radius_;
  std::unique_ptr<AnimatableFloatValue> inner_roundedness_;
  bool reversed_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_SHAPE_POLYSTAR_SHAPE_MODEL_H_
