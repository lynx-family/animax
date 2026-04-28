// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_WEB_FONT_REGISTRY_WEB_H_
#define ANIMAX_SRC_RENDER_WEB_FONT_REGISTRY_WEB_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "skity/text/font_style.hpp"

namespace skity {
class Typeface;
}  // namespace skity

namespace lynx {
namespace animax {

class FontRegistryWeb {
 public:
  static FontRegistryWeb& Get();

  bool RegisterFontData(const std::string& family_name, const void* bytes,
                        size_t length, bool is_default, int fallback_priority);

  std::shared_ptr<skity::Typeface> MatchFamilyStyle(
      const char* family_name, const skity::FontStyle& style) const;

  std::shared_ptr<skity::Typeface> MatchFamilyStyleCharacter(
      const char* family_name, const skity::FontStyle& style,
      uint32_t character) const;

  std::shared_ptr<skity::Typeface> GetDefaultTypeface(
      const skity::FontStyle& style) const;
};

}  // namespace animax
}  // namespace lynx

extern "C" skity::Typeface* AnimaXWebFontMatchFamilyStyle(
    const char* family_name, int weight, int width, int slant);

extern "C" skity::Typeface* AnimaXWebFontMatchFamilyStyleCharacter(
    const char* family_name, int weight, int width, int slant,
    uint32_t character);

#endif  // ANIMAX_SRC_RENDER_WEB_FONT_REGISTRY_WEB_H_
