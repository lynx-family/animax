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

#include "src/parser/effect/drop_shadow_effect_parser.h"

#include "src/parser/animatable/animatable_value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<DropShadowEffectModel> DropShadowEffectParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  std::unique_ptr<AnimatableColorValue> color;
  std::unique_ptr<AnimatableFloatValue> opacity;
  std::unique_ptr<AnimatableFloatValue> direction;
  std::unique_ptr<AnimatableFloatValue> distance;
  std::unique_ptr<AnimatableFloatValue> radius;

  const auto& array = value.GetArray();
  for (auto array_it = array.Begin(); array_it != array.End(); array_it++) {
    std::string cur_effect_name;
    const auto& effect_object = array_it->GetObject();
    for (auto effect_it = effect_object.MemberBegin();
         effect_it != effect_object.MemberEnd(); effect_it++) {
      const auto& effect_name = effect_it->name.GetString();
      if (strcmp("nm", effect_name) == 0) {
        cur_effect_name = effect_it->value.GetString();
      } else if (strcmp("v", effect_name) == 0) {
        if (strcmp(cur_effect_name.c_str(), "Shadow Color") == 0) {
          color =
              AnimatableValueParser::ParseColor(effect_it->value, composition);
        } else if (strcmp(cur_effect_name.c_str(), "Opacity") == 0) {
          opacity = AnimatableValueParser::ParseFloat(effect_it->value,
                                                      composition, false);
        } else if (strcmp(cur_effect_name.c_str(), "Direction") == 0) {
          direction = AnimatableValueParser::ParseFloat(effect_it->value,
                                                        composition, false);
        } else if (strcmp(cur_effect_name.c_str(), "Distance") == 0) {
          distance =
              AnimatableValueParser::ParseFloat(effect_it->value, composition);
        } else if (strcmp(cur_effect_name.c_str(), "Softness") == 0) {
          radius =
              AnimatableValueParser::ParseFloat(effect_it->value, composition);
        }
      }
    }
  }

  if (color && opacity && direction && distance && radius) {
    return std::make_unique<DropShadowEffectModel>(
        std::move(color), std::move(opacity), std::move(direction),
        std::move(distance), std::move(radius));
  }
  return nullptr;
}

}  // namespace animax
}  // namespace lynx
