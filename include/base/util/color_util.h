// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_BASE_UTIL_COLOR_UTIL_H_
#define ANIMAX_INCLUDE_BASE_UTIL_COLOR_UTIL_H_

#include <string>

#include "include/base/macros.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT ColorUtil {
 public:
  // Parses a hex color string (e.g., "#RRGGBB" or "#AARRGGBB") into an ARGB
  // int32_t value. Returns the parsed color value in 0xAARRGGBB format.
  static int32_t ParseHexColor(const std::string& color_string);

  ColorUtil() = delete;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_BASE_UTIL_COLOR_UTIL_H_
