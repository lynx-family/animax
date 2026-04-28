// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/font_typeface_provider.h"

namespace lynx {
namespace animax {

std::shared_ptr<skity::Typeface> FontTypefaceProvider::MatchFamilyStyle(
    const char* family_name, const skity::FontStyle& style) {
  return nullptr;
}

std::shared_ptr<skity::Typeface>
FontTypefaceProvider::MatchFamilyStyleCharacter(const char* family_name,
                                                const skity::FontStyle& style,
                                                skity::Unichar character) {
  return nullptr;
}

std::shared_ptr<skity::Typeface> FontTypefaceProvider::GetDefaultTypeface(
    const skity::FontStyle& style) {
  return nullptr;
}

}  // namespace animax
}  // namespace lynx
