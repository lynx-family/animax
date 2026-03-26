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

#include "src/parser/shape/polystar_shape_parser.h"

#include <string>

#include "src/model/animatable/base_animatable_value.h"
#include "src/parser/animatable/animatable_path_value_parser.h"
#include "src/parser/animatable/animatable_value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<PolystarShapeModel> PolystarShapeParser::Parse(
    rapidjson::Value& value, CompositionModel& composition, int32_t d) {
  std::string name;
  int32_t type_int = 0;
  std::unique_ptr<AnimatableFloatValue> points;
  std::unique_ptr<AnimatablePointValue> position;
  std::unique_ptr<AnimatableFloatValue> rotation;
  std::unique_ptr<AnimatableFloatValue> outer_radius;
  std::unique_ptr<AnimatableFloatValue> outer_roundedness;
  std::unique_ptr<AnimatableFloatValue> inner_radius;
  std::unique_ptr<AnimatableFloatValue> inner_roundedness;
  bool hidden = false;
  bool reversed = d == 3;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "nm") == 0) {
      name = it->value.GetString();
    } else if (strcmp(key, "sy") == 0) {
      type_int = it->value.GetInt();
    } else if (strcmp(key, "pt") == 0) {
      points = AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "p") == 0) {
      position =
          AnimatablePathValueParser::ParseSplitPath(it->value, composition);
    } else if (strcmp(key, "r") == 0) {
      rotation =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "or") == 0) {
      outer_radius = AnimatableValueParser::ParseFloat(it->value, composition);
    } else if (strcmp(key, "os") == 0) {
      outer_roundedness =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "ir") == 0) {
      inner_radius = AnimatableValueParser::ParseFloat(it->value, composition);
    } else if (strcmp(key, "is") == 0) {
      inner_roundedness =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "hd") == 0) {
      hidden = it->value.GetBool();
    } else if (strcmp(key, "d") == 0) {
      reversed = it->value.GetInt() == 3;
    }
  }

  return std::make_unique<PolystarShapeModel>(
      std::move(name), type_int, std::move(points), std::move(position),
      std::move(rotation), std::move(outer_radius),
      std::move(outer_roundedness), std::move(inner_radius),
      std::move(inner_roundedness), hidden, reversed);
}

}  // namespace animax
}  // namespace lynx
