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

#ifndef ANIMAX_SRC_MODEL_SHAPE_SHAPE_FILL_MODEL_H_
#define ANIMAX_SRC_MODEL_SHAPE_SHAPE_FILL_MODEL_H_

#include <string>

#include "src/content/shape/fill_content.h"
#include "src/model/animatable/base_animatable_value.h"
#include "src/model/content_model.h"

namespace lynx {
namespace animax {

class ShapeFillModel : public ContentModel {
 public:
  ShapeFillModel(std::string name, bool fill_enabled, int fill_type_int,
                 bool hidden, std::unique_ptr<AnimatableIntegerValue> opacity,
                 std::unique_ptr<AnimatableColorValue> color)
      : fill_enabled_(fill_enabled),
        opacity_(std::move(opacity)),
        color_(std::move(color)) {
    name_ = std::move(name);
    hidden_ = hidden;
    fill_type_ = static_cast<PathFillType>(fill_type_int - 1);
  }

  std::unique_ptr<Content> CreateContent(CompositionModel& composition,
                                         BaseLayer& layer) override;

  PathFillType GetFillType() { return fill_type_; }

  AnimatableColorValue* GetColor() { return color_.get(); }
  AnimatableIntegerValue* GetOpacity() { return opacity_.get(); }

 private:
  PathFillType fill_type_;
  [[maybe_unused]] bool fill_enabled_ = false;
  std::unique_ptr<AnimatableIntegerValue> opacity_;
  std::unique_ptr<AnimatableColorValue> color_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_SHAPE_SHAPE_FILL_MODEL_H_
