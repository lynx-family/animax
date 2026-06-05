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
  std::unique_ptr<AnimatableTextRangeSelector> text_range_selector;
  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "a") == 0) {
      anim = ParseAnimatableTextProperty(it->value, composition);
    } else if (strcmp(key, "s") == 0) {
      text_range_selector =
          ParseAnimatableTextRangeSelector(it->value, composition);
    }
  }
  if (anim == nullptr) {
    anim =
        std::unique_ptr<AnimatableTextProperty>(new AnimatableTextProperty());
  }
  if (text_range_selector != nullptr) {
    anim->SetRangeSelector(std::move(text_range_selector));
  }
  return anim;
}

std::unique_ptr<AnimatableTextProperty>
AnimatableTextPropertiesParser::ParseAnimatableTextProperty(
    rapidjson::Value& value, CompositionModel& composition) {
  if (!value.IsObject()) {
    return nullptr;
  }

  std::unique_ptr<AnimatableColorValue> color;
  std::unique_ptr<AnimatableColorValue> stroke;
  std::unique_ptr<AnimatableFloatValue> stroke_width;
  std::unique_ptr<AnimatableFloatValue> tracking;
  std::unique_ptr<AnimatableFloatValue> skew;

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
    } else if (strcmp(key, "sk") == 0) {
      skew = AnimatableValueParser::ParseFloat(it->value, composition, false);
    }
  }
  return std::make_unique<AnimatableTextProperty>(
      std::move(color), std::move(stroke), std::move(stroke_width),
      std::move(tracking), std::move(skew));
}

std::unique_ptr<AnimatableTextRangeSelector>
AnimatableTextPropertiesParser::ParseAnimatableTextRangeSelector(
    rapidjson::Value& value, CompositionModel& composition) {
  TextRangeUnits range_units = TextRangeUnits::kPercent;
  std::unique_ptr<AnimatableFloatValue> offset;
  std::unique_ptr<AnimatableFloatValue> start;
  std::unique_ptr<AnimatableFloatValue> end;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "o") == 0) {
      offset = AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "r") == 0) {
      if (it->value.IsInt()) {
        const auto units = it->value.GetInt();
        if (units == static_cast<int>(TextRangeUnits::kPercent) ||
            units == static_cast<int>(TextRangeUnits::kIndex)) {
          range_units = static_cast<TextRangeUnits>(units);
        }
      }
    } else if (strcmp(key, "s") == 0) {
      start = AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "e") == 0) {
      end = AnimatableValueParser::ParseFloat(it->value, composition, false);
    }
  }
  return std::make_unique<AnimatableTextRangeSelector>(
      range_units, std::move(offset), std::move(start), std::move(end));
}

}  // namespace animax
}  // namespace lynx
