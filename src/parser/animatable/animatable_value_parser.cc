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

#include "src/parser/animatable/animatable_value_parser.h"

#include <memory>

namespace lynx {
namespace animax {

std::unique_ptr<AnimatableShapeValue> AnimatableValueParser::ParseShapeData(
    rapidjson::Value& value, CompositionModel& composition) {
  auto shape_value =
      std::unique_ptr<AnimatableShapeValue>(new AnimatableShapeValue());
  Parse(value, composition, composition.GetScale(), shape_value->GetKeyframes(),
        ValueType::kShape);
  return shape_value;
}

std::unique_ptr<AnimatableFloatValue> AnimatableValueParser::ParseFloat(
    rapidjson::Value& value, CompositionModel& composition) {
  return ParseFloat(value, composition, true);
}

std::unique_ptr<AnimatableFloatValue> AnimatableValueParser::ParseFloat(
    rapidjson::Value& value, CompositionModel& composition, bool is_dp) {
  auto anim_value =
      std::unique_ptr<AnimatableFloatValue>(new AnimatableFloatValue());
  auto scale = is_dp ? composition.GetScale() : 1;
  Parse(value, composition, scale, anim_value->GetKeyframes(),
        ValueType::kFloat);
  return anim_value;
}

std::unique_ptr<AnimatableIntegerValue> AnimatableValueParser::ParseInteger(
    rapidjson::Value& value, CompositionModel& composition) {
  auto anim_value =
      std::unique_ptr<AnimatableIntegerValue>(new AnimatableIntegerValue());
  Parse(value, composition, anim_value->GetKeyframes(), ValueType::kInteger);
  return anim_value;
}

std::unique_ptr<AnimatablePointValue> AnimatableValueParser::ParsePoint(
    rapidjson::Value& value, CompositionModel& composition) {
  auto anim_value =
      std::unique_ptr<AnimatablePointValue>(new AnimatablePointValue());
  Parse(value, composition, composition.GetScale(), anim_value->GetKeyframes(),
        ValueType::kPoint);
  return anim_value;
}

std::unique_ptr<AnimatableScaleValue> AnimatableValueParser::ParseScale(
    rapidjson::Value& value, CompositionModel& composition) {
  auto anim_value =
      std::unique_ptr<AnimatableScaleValue>(new AnimatableScaleValue());
  Parse(value, composition, anim_value->GetKeyframes(), ValueType::kScale);
  return anim_value;
}

std::unique_ptr<AnimatableColorValue> AnimatableValueParser::ParseColor(
    rapidjson::Value& value, CompositionModel& composition) {
  auto anim_value =
      std::unique_ptr<AnimatableColorValue>(new AnimatableColorValue());
  Parse(value, composition, anim_value->GetKeyframes(), ValueType::kColor);
  return anim_value;
}

std::unique_ptr<AnimatableTextFrame> AnimatableValueParser::ParseDocumentData(
    rapidjson::Value& value, CompositionModel& composition) {
  auto anim_value =
      std::unique_ptr<AnimatableTextFrame>(new AnimatableTextFrame());
  Parse(value, composition, composition.GetScale(), anim_value->GetKeyframes(),
        ValueType::kDocument);
  return anim_value;
}

std::unique_ptr<AnimatableGradientValue>
AnimatableValueParser::ParseGradientColor(rapidjson::Value& value,
                                          CompositionModel& composition) {
  auto anim_value =
      std::unique_ptr<AnimatableGradientValue>(new AnimatableGradientValue());
  Parse(value, composition, anim_value->GetKeyframes(), ValueType::kGradient);
  return anim_value;
}

}  // namespace animax
}  // namespace lynx
