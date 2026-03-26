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

#ifndef ANIMAX_SRC_MODEL_TEXT_FONT_CHARACTER_MODEL_H_
#define ANIMAX_SRC_MODEL_TEXT_FONT_CHARACTER_MODEL_H_

#include <memory>

#include "src/model/shape/shape_group_model.h"

namespace lynx {
namespace animax {

class FontCharacterModel {
 public:
  FontCharacterModel() = default;
  void Init(char character, float size, float width, std::string style,
            std::string font_family) {
    character_ = character;
    size_ = size;
    width_ = width;
    style_ = std::move(style);
    font_family_ = std::move(font_family);
  }

  std::vector<std::unique_ptr<ContentModel>>& GetShapes() { return shapes_; }
  float GetWidth() { return width_; }
  int32_t HashCode() { return HashFor(character_, font_family_, style_); }
  static int32_t HashFor(const char& character, const std::string& font_family,
                         const std::string& style) {
    int32_t result = static_cast<int32_t>(character);
    result = 31 * result + std::hash<std::string>{}(font_family);
    result = 31 * result + std::hash<std::string>{}(style);
    return result;
  }

 private:
  std::vector<std::unique_ptr<ContentModel>> shapes_;
  char character_ = '\0';
  float size_ = 0;
  float width_ = 0;
  std::string style_;
  std::string font_family_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_TEXT_FONT_CHARACTER_MODEL_H_
