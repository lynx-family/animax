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

#include "src/parser/animatable/animatable_path_value_parser.h"

#include <memory>
#include <vector>

#include "src/model/animatable/base_animatable_value.h"
#include "src/parser/animatable/animatable_value_parser.h"
#include "src/parser/keyframe/json_parser.h"
#include "src/parser/keyframe/path_keyframe_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<AnimatablePathValue> AnimatablePathValueParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  auto anim_value =
      std::unique_ptr<AnimatablePathValue>(new AnimatablePathValue());
  auto& keyframes = anim_value->GetKeyframes();
  if (value.IsArray()) {
    const auto& array = value.GetArray();
    if ((array.Size() == 2 || array.Size() == 3) && array[0].IsNumber()) {
      keyframes.emplace_back(
          PathKeyframeParser::ParseStaticValue(value, composition));
    } else {
      for (auto it = array.Begin(); it != array.End(); it++) {
        keyframes.emplace_back(
            PathKeyframeParser::Parse(it->Move(), composition));
      }
    }
    KeyframesParser::SetEndFrames(keyframes);
  } else {
    keyframes.emplace_back(std::make_unique<KeyframeModel>(
        composition, JsonParser::JsonToPoint(value, composition.GetScale())));
  }

  if (keyframes.empty()) {
    ANIMAX_LOGI("Found empty keyframe list on AnimatablePathValue.");
  }

  return anim_value;
}

std::unique_ptr<AnimatablePointValue> AnimatablePathValueParser::ParseSplitPath(
    rapidjson::Value& value, CompositionModel& composition) {
  std::unique_ptr<AnimatablePathValue> path_anim;
  std::unique_ptr<AnimatableFloatValue> x_anim;
  std::unique_ptr<AnimatableFloatValue> y_anim;
  std::unique_ptr<AnimatableFloatValue> z_anim;

  bool has_expressions = false;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "k") == 0) {
      path_anim = AnimatablePathValueParser::Parse(it->value, composition);
    } else if (strcmp(key, "x") == 0) {
      if (it->value.IsString()) {
        has_expressions = true;
      } else {
        x_anim = AnimatableValueParser::ParseFloat(it->value, composition);
      }
    } else if (strcmp(key, "y") == 0) {
      if (it->value.IsString()) {
        has_expressions = true;
      } else {
        y_anim = AnimatableValueParser::ParseFloat(it->value, composition);
      }
    } else if (strcmp(key, "z") == 0) {
      if (it->value.IsString()) {
        has_expressions = true;
      } else {
        z_anim = AnimatableValueParser::ParseFloat(it->value, composition);
      }
    }
  }

  if (has_expressions) {
    ANIMAX_LOGI("path value parser do not support expression");
  }

  if (path_anim) {
    return path_anim;
  }

  return std::unique_ptr<AnimatableSplitDimensionPathValue>(
      new AnimatableSplitDimensionPathValue(
          std::move(x_anim), std::move(y_anim), std::move(z_anim)));
}

}  // namespace animax
}  // namespace lynx
