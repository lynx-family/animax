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

#include "src/parser/animatable/animatable_text_properties_parser.h"

#include <memory>

#include "src/parser/animatable/animatable_value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<AnimatableTextProperty> AnimatableTextPropertiesParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  std::unique_ptr<AnimatableTextProperty> anim;
  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "a") == 0) {
      anim = ParseAnimatableTextProperty(it->value, composition);
    }
  }
  if (anim == nullptr) {
    return std::unique_ptr<AnimatableTextProperty>(
        new AnimatableTextProperty());
  }
  return anim;
}

std::unique_ptr<AnimatableTextProperty>
AnimatableTextPropertiesParser::ParseAnimatableTextProperty(
    rapidjson::Value& value, CompositionModel& composition) {
  std::unique_ptr<AnimatableColorValue> color;
  std::unique_ptr<AnimatableColorValue> stroke;
  std::unique_ptr<AnimatableFloatValue> stroke_width;
  std::unique_ptr<AnimatableFloatValue> tracking;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "fc") == 0) {
      color = AnimatableValueParser::ParseColor(it->value, composition);
    } else if (strcmp(key, "sc") == 0) {
      stroke = AnimatableValueParser::ParseColor(it->value, composition);
    } else if (strcmp(key, "sw") == 0) {
      stroke_width = AnimatableValueParser::ParseFloat(it->value, composition);
    } else if (strcmp(key, "t") == 0) {
      tracking = AnimatableValueParser::ParseFloat(it->value, composition);
    }
  }
  return std::make_unique<AnimatableTextProperty>(
      std::move(color), std::move(stroke), std::move(stroke_width),
      std::move(tracking));
}

}  // namespace animax
}  // namespace lynx
