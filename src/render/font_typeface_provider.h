// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_FONT_TYPEFACE_PROVIDER_H_
#define ANIMAX_SRC_RENDER_FONT_TYPEFACE_PROVIDER_H_

#include <memory>

#include "skity/text/font_style.hpp"
#include "skity/text/glyph.hpp"

namespace skity {
class Typeface;
}  // namespace skity

namespace lynx {
namespace animax {

class FontTypefaceProvider {
 public:
  static std::shared_ptr<skity::Typeface> MatchFamilyStyle(
      const char* family_name, const skity::FontStyle& style);

  static std::shared_ptr<skity::Typeface> MatchFamilyStyleCharacter(
      const char* family_name, const skity::FontStyle& style,
      skity::Unichar character);

  static std::shared_ptr<skity::Typeface> GetDefaultTypeface(
      const skity::FontStyle& style);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_FONT_TYPEFACE_PROVIDER_H_
