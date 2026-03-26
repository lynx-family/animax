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

#ifndef ANIMAX_SRC_MODEL_MASK_MODEL_H_
#define ANIMAX_SRC_MODEL_MASK_MODEL_H_

#include <memory>

#include "src/model/animatable/base_animatable_value.h"

namespace lynx {
namespace animax {

enum class MaskMode : uint8_t { kAdd = 0, kSubtract, kIntersect, kNone };

class MaskKeyframeAnimation;
class BaseLayer;

class MaskModel {
 public:
  MaskModel(MaskMode mask_mode, std::unique_ptr<AnimatableShapeValue> mask_path,
            std::unique_ptr<AnimatableIntegerValue> opacity, bool inverted)
      : mask_mode_(mask_mode),
        mask_path_(std::move(mask_path)),
        opacity_(std::move(opacity)),
        inverted_(inverted) {}

 private:
  friend class MaskKeyframeAnimation;
  friend class BaseLayer;

  MaskMode mask_mode_;
  std::unique_ptr<AnimatableShapeValue> mask_path_;
  std::unique_ptr<AnimatableIntegerValue> opacity_;
  bool inverted_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_MASK_MODEL_H_
