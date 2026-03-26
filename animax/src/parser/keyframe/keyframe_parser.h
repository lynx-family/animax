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

#ifndef ANIMAX_SRC_PARSER_KEYFRAME_KEYFRAME_PARSER_H_
#define ANIMAX_SRC_PARSER_KEYFRAME_KEYFRAME_PARSER_H_

#include <vector>

#include "src/animation/interpolator/interpolator.h"
#include "src/model/composition_model.h"
#include "src/model/keyframe/keyframe_model.h"
#include "src/parser/keyframe/json_parser.h"
#include "third_party/rapidjson/document.h"

namespace lynx {
namespace animax {

class KeyframeParser {
 public:
  static std::unique_ptr<KeyframeModel> Parse(rapidjson::Value& value,
                                              CompositionModel& composition,
                                              float scale, bool animated,
                                              bool multi_dimen, ValueType type);

  static std::unique_ptr<KeyframeModel> ParseMultiDimensionalKeyframe(
      rapidjson::Value& value, CompositionModel& composition, float scale,
      ValueType type);

  static void ParseControlPoints(rapidjson::Value& value, PointF& x_cp,
                                 PointF& y_cp);

  static std::unique_ptr<KeyframeModel> ParseKeyframe(
      rapidjson::Value& value, CompositionModel& composition, float scale,
      ValueType type);

  static std::unique_ptr<KeyframeModel> ParseStaticValue(
      rapidjson::Value& value, CompositionModel& composition, float scale,
      ValueType type);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PARSER_KEYFRAME_KEYFRAME_PARSER_H_
