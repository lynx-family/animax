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

#include "src/parser/shape/shape_stroke_parser.h"

#include <string>

#include "src/model/animatable/base_animatable_value.h"
#include "src/parser/animatable/animatable_value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<ShapeStrokeModel> ShapeStrokeParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  std::string name;
  std::unique_ptr<AnimatableColorValue> color;
  std::unique_ptr<AnimatableFloatValue> width;
  std::unique_ptr<AnimatableIntegerValue> opacity;
  int32_t cap_type_int = 0;
  int32_t join_type_int = 0;
  std::shared_ptr<AnimatableFloatValue> offset;
  float miter_limit = 0;
  bool hidden = false;

  auto shape_stroke = std::unique_ptr<ShapeStrokeModel>(new ShapeStrokeModel());
  auto& line_dash_pattern = shape_stroke->GetLineDashPattern();

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "nm") == 0) {
      name = it->value.GetString();
    } else if (strcmp(key, "c") == 0) {
      color = AnimatableValueParser::ParseColor(it->value, composition);
    } else if (strcmp(key, "w") == 0) {
      width = AnimatableValueParser::ParseFloat(it->value, composition);
    } else if (strcmp(key, "o") == 0) {
      opacity = AnimatableValueParser::ParseInteger(it->value, composition);
    } else if (strcmp(key, "lc") == 0) {
      cap_type_int = it->value.GetInt();
    } else if (strcmp(key, "lj") == 0) {
      join_type_int = it->value.GetInt();
    } else if (strcmp(key, "ml") == 0) {
      miter_limit = it->value.GetFloat();
    } else if (strcmp(key, "hd") == 0) {
      hidden = it->value.GetBool();
    } else if (strcmp(key, "d") == 0) {
      const auto& array = it->value.GetArray();
      auto dash_name = "";
      std::shared_ptr<AnimatableFloatValue> dash_value;
      for (auto array_it = array.Begin(); array_it != array.End(); array_it++) {
        const auto& array_object = array_it->GetObject();
        for (auto object_it = array_object.MemberBegin();
             object_it != array_object.MemberEnd(); object_it++) {
          const auto& object_key = object_it->name.GetString();
          if (strcmp(object_key, "n") == 0) {
            dash_name = object_it->value.GetString();
          } else if (strcmp(object_key, "v") == 0) {
            dash_value = AnimatableValueParser::ParseFloat(object_it->value,
                                                           composition);
          }
        }

        if (strcmp(dash_name, "o") == 0) {
          offset = dash_value;
        } else if (strcmp(dash_name, "d") == 0 || strcmp(dash_name, "g") == 0) {
          composition.SetHashDashPattern(true);
          if (dash_value) {
            line_dash_pattern.push_back(dash_value);
          }
        }
      }

      if (line_dash_pattern.size() == 1) {
        line_dash_pattern.push_back(line_dash_pattern[0]);
      }
    }
  }

  if (opacity == nullptr) {
    opacity = AnimatableIntegerValue::MakeDefault(composition);
  }

  shape_stroke->Init(std::move(name), std::move(offset), std::move(color),
                     std::move(opacity), std::move(width), cap_type_int,
                     join_type_int, miter_limit, hidden);
  return shape_stroke;
}

}  // namespace animax
}  // namespace lynx
