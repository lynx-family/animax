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

#include "src/parser/path/merge_paths_parser.h"

#include <string>

#include "src/model/path/merge_paths_model.h"

namespace lynx {
namespace animax {

std::unique_ptr<MergePathsModel> MergePathsParser::Parse(
    rapidjson::Value& value) {
  std::string name;
  int32_t mode_int = 0;
  bool hidden = false;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "nm") == 0) {
      name = it->value.GetString();
    } else if (strcmp(key, "mm") == 0) {
      mode_int = it->value.GetInt();
    } else if (strcmp(key, "hd") == 0) {
      hidden = it->value.GetBool();
    }
  }

  return std::unique_ptr<MergePathsModel>(
      new MergePathsModel(std::move(name), mode_int, hidden));
}

}  // namespace animax
}  // namespace lynx
