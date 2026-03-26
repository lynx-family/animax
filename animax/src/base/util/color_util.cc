// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/base/util/color_util.h"

#include <string>

namespace {
constexpr int kHexSplitLength = 2;
}  // namespace

namespace lynx {
namespace animax {

int32_t ColorUtil::ParseHexColor(const std::string& color_string) {
  if (color_string.empty()) {
    return 0;
  }

  std::string color = color_string;
  if (color[0] == '#') {
    color.erase(0, 1);
  }

  if (color.length() == 6) {
    color = "ff" + color;
  } else if (color.length() != 8) {
    return 0;
  }

  const int num_sub_color = color.length() / kHexSplitLength;

  int32_t a = 255, r = 0, g = 0, b = 0;
  for (int i = 0; i < num_sub_color && i < 4; i++) {
    const auto sub_color = color.substr(i * kHexSplitLength, kHexSplitLength);
    char* end = nullptr;
    const long value = strtol(sub_color.c_str(), &end, 16);
    if (end == nullptr || *end != '\0') {
      return 0;
    }
    switch (i) {
      case 0:
        a = static_cast<int32_t>(value);
        break;
      case 1:
        r = static_cast<int32_t>(value);
        break;
      case 2:
        g = static_cast<int32_t>(value);
        break;
      case 3:
        b = static_cast<int32_t>(value);
        break;
      default:
        break;
    }
  }

  return (a << 24) | (r << 16) | (g << 8) | b;
}

}  // namespace animax
}  // namespace lynx
