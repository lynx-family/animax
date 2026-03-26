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

#include "src/parser/text/font_parser.h"

#include "src/resource/asset/font_asset.h"

namespace lynx {
namespace animax {

std::shared_ptr<FontAsset> FontParser::Parse(rapidjson::Value& value,
                                             CompositionModel& composition) {
  auto info = FontAssetModel{};

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "fFamily") == 0) {
      info.family = it->value.GetString();
    } else if (strcmp(key, "fName") == 0) {
      info.name = it->value.GetString();
    } else if (strcmp(key, "fStyle") == 0) {
      info.style = it->value.GetString();
    } else if (strcmp(key, "ascent") == 0) {
      info.ascent = it->value.GetFloat();
    } else if (strcmp(key, "fPath") == 0) {
      info.path = it->value.GetString();
    } else if (strcmp(key, "origin") == 0) {
      info.path_origin = static_cast<FontPathOrigin>(it->value.GetInt());
    }
  }

  return std::shared_ptr<FontAsset>(new FontAsset(std::move(info)));
}

}  // namespace animax
}  // namespace lynx
