// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_FONT_ASSET_MODEL_H_
#define ANIMAX_SRC_RESOURCE_ASSET_FONT_ASSET_MODEL_H_

#include <string>

namespace lynx {
namespace animax {

enum class FontPathOrigin : uint8_t {
  kLocal = 0,  // system font
  kCssUrl = 1,
  kScriptUrl = 2,
  kFontUrl = 3,
};

struct FontAssetModel {
  /**
    * {
          "origin": 0,
          "fPath": "",
          "fClass": "",
          "fFamily": "Arial",
          "fWeight": "",
          "fStyle": "Regular",
          "fName": "ArialMT",
          "ascent": 71.5988159179688
      }
   */
  std::string family{};          // fFamily
  std::string name{};            // fName
  std::string style{};           // fStyle
  float ascent = 0.f;            // ascent
  std::string path{};            // fPath
  FontPathOrigin path_origin{};  // origin
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_FONT_ASSET_MODEL_H_
