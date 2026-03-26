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

#include "src/parser/effect/blur_effect_parser.h"

#include "src/parser/animatable/animatable_value_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<BlurEffectModel> BlurEffectParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  std::unique_ptr<BlurEffectModel> model;
  const auto& array = value.GetArray();
  for (auto array_it = array.Begin(); array_it != array.End(); array_it++) {
    const auto& object = array_it->GetObject();
    bool is_correct_type = false;
    for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
      const auto& name = it->name.GetString();
      if (strcmp("ty", name) == 0) {
        is_correct_type = it->value.GetInt() == 0;
      } else if (strcmp("v", name) == 0) {
        if (is_correct_type) {
          auto blurriness =
              AnimatableValueParser::ParseFloat(it->value, composition);
          model = std::unique_ptr<BlurEffectModel>(
              new BlurEffectModel(std::move(blurriness)));
        }
      }
    }
  }
  return model;
}

}  // namespace animax
}  // namespace lynx
