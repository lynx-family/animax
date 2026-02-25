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

#include "src/parser/shape/shape_fill_parser.h"

#include "src/model/animatable/base_animatable_value.h"
#include "src/model/shape/shape_fill_model.h"
#include "src/parser/animatable/animatable_value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<ShapeFillModel> ShapeFillParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  bool fill_enabled = false;
  std::string name;
  int32_t fill_type_int = 1;
  bool hidden = false;

  std::unique_ptr<AnimatableIntegerValue> opacity;
  std::unique_ptr<AnimatableColorValue> color;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "nm") == 0) {
      name = it->value.GetString();
    } else if (strcmp(key, "c") == 0) {
      color = AnimatableValueParser::ParseColor(it->value, composition);
    } else if (strcmp(key, "o") == 0) {
      opacity = AnimatableValueParser::ParseInteger(it->value, composition);
    } else if (strcmp(key, "fillEnabled") == 0) {
      fill_enabled = it->value.GetString();
    } else if (strcmp(key, "r") == 0) {
      fill_type_int = it->value.GetInt();
    } else if (strcmp(key, "hd") == 0) {
      hidden = it->value.GetBool();
    }
  }

  if (opacity == nullptr) {
    opacity = AnimatableIntegerValue::MakeDefault(composition);
  }

  return std::unique_ptr<ShapeFillModel>(
      new ShapeFillModel(std::move(name), fill_enabled, fill_type_int, hidden,
                         std::move(opacity), std::move(color)));
}

}  // namespace animax
}  // namespace lynx
