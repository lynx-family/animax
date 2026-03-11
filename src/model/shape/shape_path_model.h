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

#ifndef ANIMAX_SRC_MODEL_SHAPE_SHAPE_PATH_MODEL_H_
#define ANIMAX_SRC_MODEL_SHAPE_SHAPE_PATH_MODEL_H_

#include <string>

#include "base/include/compiler_specific.h"
#include "src/content/shape/shape_content.h"
#include "src/model/content_model.h"

namespace lynx {
namespace animax {

class AnimatableShapeValue;

class ShapePathModel : public ContentModel {
 public:
  ShapePathModel(std::string name, int index,
                 std::unique_ptr<AnimatableShapeValue> anim_shape_value,
                 bool hidden)
      : index_(index), shape_path_(std::move(anim_shape_value)) {
    name_ = std::move(name);
    hidden_ = hidden;
  }

  std::unique_ptr<Content> CreateContent(CompositionModel& composition,
                                         BaseLayer& layer);

  AnimatableShapeValue* GetShapePath() { return shape_path_.get(); }

 private:
  ALLOW_UNUSED_TYPE int32_t index_ = 0;
  std::unique_ptr<AnimatableShapeValue> shape_path_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_SHAPE_SHAPE_PATH_MODEL_H_
