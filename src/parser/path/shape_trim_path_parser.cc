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

#include "src/parser/path/shape_trim_path_parser.h"

#include <memory>

#include "src/model/animatable/base_animatable_value.h"
#include "src/parser/animatable/animatable_value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<ShapeTrimPathModel> ShapeTrimPathParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  std::string name;
  int32_t shape_trim_type_int = 0;
  std::unique_ptr<AnimatableFloatValue> start;
  std::unique_ptr<AnimatableFloatValue> end;
  std::unique_ptr<AnimatableFloatValue> offset;
  bool hidden = false;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "s") == 0) {
      start = AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "e") == 0) {
      end = AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "o") == 0) {
      offset = AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "nm") == 0) {
      name = it->value.GetString();
    } else if (strcmp(key, "m") == 0) {
      shape_trim_type_int = it->value.GetInt();
    } else if (strcmp(key, "hd") == 0) {
      hidden = it->value.GetBool();
    }
  }

  return std::make_unique<ShapeTrimPathModel>(
      std::move(name), shape_trim_type_int, std::move(start), std::move(end),
      std::move(offset), hidden);
}

}  // namespace animax
}  // namespace lynx
