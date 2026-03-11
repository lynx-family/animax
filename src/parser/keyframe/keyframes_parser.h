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

#ifndef ANIMAX_SRC_PARSER_KEYFRAME_KEYFRAMES_PARSER_H_
#define ANIMAX_SRC_PARSER_KEYFRAME_KEYFRAMES_PARSER_H_

#include <memory>
#include <vector>

#include "src/base/log/log.h"
#include "src/model/composition_model.h"
#include "src/model/keyframe/keyframe_model.h"
#include "src/model/keyframe/path_keyframe_model.h"
#include "src/parser/keyframe/keyframe_parser.h"
#include "src/parser/value_parser.h"
#include "third_party/rapidjson/document.h"

namespace lynx {
namespace animax {

class KeyframesParser {
 public:
  static void Parse(rapidjson::Value& value, CompositionModel& composition,
                    bool multi_dimen, float scale, KeyframeModelList& keyframes,
                    ValueType type);

  static void SetEndFrames(KeyframeModelList& frames);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PARSER_KEYFRAME_KEYFRAMES_PARSER_H_
