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

#ifndef ANIMAX_SRC_PARSER_TEXT_FONT_CHARACTER_PARSER_H_
#define ANIMAX_SRC_PARSER_TEXT_FONT_CHARACTER_PARSER_H_

#include "src/model/composition_model.h"
#include "src/model/text/font_character_model.h"
#include "third_party/rapidjson/document.h"

namespace lynx {
namespace animax {

class FontCharacterParser {
 public:
  static std::unique_ptr<FontCharacterModel> Parse(
      rapidjson::Value& value, CompositionModel& composition);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PARSER_TEXT_FONT_CHARACTER_PARSER_H_
