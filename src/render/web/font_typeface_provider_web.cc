// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/font_typeface_provider.h"
#include "src/render/web/font_registry_web.h"

namespace lynx {
namespace animax {

std::shared_ptr<skity::Typeface> FontTypefaceProvider::MatchFamilyStyle(
    const char* family_name, const skity::FontStyle& style) {
  return FontRegistryWeb::Get().MatchFamilyStyle(family_name, style);
}

std::shared_ptr<skity::Typeface>
FontTypefaceProvider::MatchFamilyStyleCharacter(const char* family_name,
                                                const skity::FontStyle& style,
                                                skity::Unichar character) {
  return FontRegistryWeb::Get().MatchFamilyStyleCharacter(family_name, style,
                                                          character);
}

std::shared_ptr<skity::Typeface> FontTypefaceProvider::GetDefaultTypeface(
    const skity::FontStyle& style) {
  return FontRegistryWeb::Get().GetDefaultTypeface(style);
}

}  // namespace animax
}  // namespace lynx
