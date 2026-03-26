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

#include "src/parser/text/font_character_parser.h"

#include "src/parser/content_model_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<FontCharacterModel> FontCharacterParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  char character = '\0';
  float size = 0;
  float width = 0;
  std::string style;
  std::string font_family;

  auto character_model =
      std::unique_ptr<FontCharacterModel>(new FontCharacterModel());
  auto& shapes = character_model->GetShapes();

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "ch") == 0) {
      character = it->value.GetString()[0];
    } else if (strcmp(key, "size") == 0) {
      size = it->value.GetFloat();
    } else if (strcmp(key, "w") == 0) {
      width = it->value.GetFloat();
    } else if (strcmp(key, "style") == 0) {
      style = it->value.GetString();
    } else if (strcmp(key, "fFamily") == 0) {
      font_family = it->value.GetString();
    } else if (strcmp(key, "data") == 0 && it->value.IsObject()) {
      const auto& data_object = it->value.GetObject();
      for (auto data_it = data_object.MemberBegin();
           data_it != data_object.MemberEnd(); data_it++) {
        const auto& data_key = data_it->name.GetString();
        if (strcmp(data_key, "shapes") == 0) {
          const auto& shape_array = data_it->value.GetArray();
          for (auto shape_it = shape_array.Begin();
               shape_it != shape_array.End(); shape_it++) {
            auto group =
                ContentModelParser::Parse(shape_it->Move(), composition);
            if (group->Type() == ModelType::kShapeGroup) {
              shapes.push_back(std::move(group));
            }
          }
        }
      }
    }
  }

  character_model->Init(character, size, width, style, font_family);
  return character_model;
}

}  // namespace animax
}  // namespace lynx
