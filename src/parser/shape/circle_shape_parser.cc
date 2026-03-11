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

#include "src/parser/shape/circle_shape_parser.h"

#include <memory>
#include <string>

#include "src/model/animatable/animatable_value.h"
#include "src/model/animatable/base_animatable_value.h"
#include "src/parser/animatable/animatable_path_value_parser.h"
#include "src/parser/animatable/animatable_value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<CircleShapeModel> CircleShapeParser::Parse(
    rapidjson::Value& value, CompositionModel& composition, int32_t d) {
  std::string name;
  std::unique_ptr<AnimatablePointValue> position;
  std::unique_ptr<AnimatablePointValue> size;
  bool reversed = d == 3;
  bool hidden = false;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "nm") == 0) {
      name = it->value.GetString();
    } else if (strcmp(key, "p") == 0) {
      position =
          AnimatablePathValueParser::ParseSplitPath(it->value, composition);
    } else if (strcmp(key, "s") == 0) {
      size = AnimatableValueParser::ParsePoint(it->value, composition);
    } else if (strcmp(key, "hd") == 0) {
      hidden = it->value.GetBool();
    } else if (strcmp(key, "d") == 0) {
      reversed = it->value.GetInt() == 3;
    }
  }

  return std::make_unique<CircleShapeModel>(
      std::move(name), std::move(position), std::move(size), reversed, hidden);
}

}  // namespace animax
}  // namespace lynx
