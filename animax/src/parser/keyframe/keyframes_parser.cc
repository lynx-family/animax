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

#include "src/parser/keyframe/keyframes_parser.h"

namespace lynx {
namespace animax {

void KeyframesParser::Parse(rapidjson::Value& value,
                            CompositionModel& composition, bool multi_dimen,
                            float scale, KeyframeModelList& keyframes,
                            ValueType type) {
  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    if (strcmp(it->name.GetString(), "k") == 0) {
      if (it->value.IsArray()) {
        const auto& value_array = it->value.GetArray();
        if (value_array.Empty()) {
          ANIMAX_LOGI("KeyframesParser array is empty");
          break;
        }

        if (value_array.Begin()->IsNumber()) {
          keyframes.emplace_back(KeyframeParser::Parse(
              it->value, composition, scale, false, multi_dimen, type));
        } else {
          for (auto array_it = value_array.Begin();
               array_it != value_array.End(); array_it++) {
            keyframes.emplace_back(KeyframeParser::Parse(
                array_it->Move(), composition, scale, true, multi_dimen, type));
          }
        }
      } else {
        keyframes.emplace_back(KeyframeParser::Parse(
            it->value, composition, scale, false, multi_dimen, type));
      }
    }
  }

  SetEndFrames(keyframes);
}

void KeyframesParser::SetEndFrames(KeyframeModelList& frames) {
  auto size = frames.size();
  if (size == 0) {
    return;
  }

  for (auto i = 0; i < size - 1; i++) {
    auto& keyframe = frames[i];
    auto& next_keyframe = frames[i + 1];
    keyframe->SetEndFrame(next_keyframe->GetStartFrame());
    if (keyframe->IsEndValueEmpty() && !next_keyframe->IsStartValueEmpty()) {
      keyframe->SetEndValue(next_keyframe->CopyStartValue());
    }
  }

  auto& last_frame = frames[size - 1];
  if ((last_frame->IsStartValueEmpty() || last_frame->IsEndValueEmpty()) &&
      size > 1) {
    frames.pop_back();
  }
}

}  // namespace animax
}  // namespace lynx
