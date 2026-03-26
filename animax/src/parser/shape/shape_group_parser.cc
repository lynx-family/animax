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

#include "src/parser/shape/shape_group_parser.h"

#include <memory>
#include <vector>

#include "src/model/content_model.h"
#include "src/parser/content_model_parser.h"

namespace lynx {
namespace animax {

std::unique_ptr<ShapeGroupModel> ShapeGroupParser::Parse(
    rapidjson::Value& value, CompositionModel& composition) {
  std::string name;
  bool hidden = false;
  auto model = std::unique_ptr<ShapeGroupModel>(new ShapeGroupModel());

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "nm") == 0) {
      name = it->value.GetString();
    } else if (strcmp(key, "hd") == 0) {
      hidden = it->value.GetBool();
    } else if (strcmp(key, "it") == 0) {
      const auto& array = it->value.GetArray();
      for (auto content_it = array.Begin(); content_it != array.End();
           content_it++) {
        auto content_model =
            ContentModelParser::Parse(content_it->Move(), composition);
        if (content_model) {
          model->AddOwnItem(std::move(content_model));
        }
      }
    }
  }

  model->name_ = std::move(name);
  model->hidden_ = hidden;

  return model;
}

}  // namespace animax
}  // namespace lynx
