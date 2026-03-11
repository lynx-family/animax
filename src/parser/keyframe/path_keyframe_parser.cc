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

#include "src/parser/keyframe/path_keyframe_parser.h"

#include "src/parser/keyframe/keyframe_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<PathKeyframeModel> PathKeyframeParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  bool animated = value.IsObject();
  auto keyframe =
      KeyframeParser::Parse(value, composition, composition.GetScale(),
                            animated, false, ValueType::kPath);
  return std::unique_ptr<PathKeyframeModel>(
      new PathKeyframeModel(composition, *keyframe));
}

std::unique_ptr<PathKeyframeModel> PathKeyframeParser::ParseStaticValue(
    rapidjson::Value& value, CompositionModel& composition) {
  auto keyframe = KeyframeParser::ParseStaticValue(
      value, composition, composition.GetScale(), ValueType::kPath);
  return std::unique_ptr<PathKeyframeModel>(
      new PathKeyframeModel(composition, *keyframe));
}

}  // namespace animax
}  // namespace lynx
