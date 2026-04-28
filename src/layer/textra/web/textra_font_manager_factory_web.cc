// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "skity/text/typeface.hpp"
#include "src/base/log/log.h"
#include "src/layer/textra/textra_font_manager_factory.h"
#include "src/render/web/font_registry_web.h"

namespace lynx {
namespace animax {
namespace {

class TTAnimaXWebFontManager : public TTSkityFontManager {
 public:
  TypefaceRef matchFamilyStyle(
      const char* family_name,
      const ttoffice::tttext::FontStyle& style) override {
    auto typeface = WrapWebTypeface(
        AnimaXWebFontMatchFamilyStyle(family_name, style.GetWeight(),
                                      style.GetWidth(), style.GetSlant()),
        "matchFamilyStyle", family_name);
    return typeface ? typeface
                    : TTSkityFontManager::matchFamilyStyle(family_name, style);
  }

  TypefaceRef matchFamilyStyleCharacter(
      const char* family_name, const ttoffice::tttext::FontStyle& style,
      const char** bcp47, int bcp47_count, uint32_t character) override {
    auto typeface =
        WrapWebTypeface(AnimaXWebFontMatchFamilyStyleCharacter(
                            family_name, style.GetWeight(), style.GetWidth(),
                            style.GetSlant(), character),
                        "matchFamilyStyleCharacter", family_name);
    return typeface ? typeface
                    : TTSkityFontManager::matchFamilyStyleCharacter(
                          family_name, style, bcp47, bcp47_count, character);
  }

  TypefaceRef legacyMakeTypeface(
      const char* family_name,
      ttoffice::tttext::FontStyle style) const override {
    auto typeface = WrapWebTypeface(
        AnimaXWebFontMatchFamilyStyle(family_name, style.GetWeight(),
                                      style.GetWidth(), style.GetSlant()),
        "legacyMakeTypeface", family_name);
    return typeface
               ? typeface
               : TTSkityFontManager::legacyMakeTypeface(family_name, style);
  }

 private:
  static TypefaceRef WrapWebTypeface(skity::Typeface* typeface,
                                     const char* api_name,
                                     const char* family_name) {
    if (typeface == nullptr) {
      ANIMAX_LOGE("TTAnimaXWebFontManager::"
                  << api_name << " failed, web typeface is null, family_name: "
                  << (family_name == nullptr ? "" : family_name));
      return nullptr;
    }

    auto typeface_ref =
        std::shared_ptr<skity::Typeface>(typeface, [](skity::Typeface*) {});
    return std::make_shared<skity::textlayout::SkityTypefaceHelper>(
        std::move(typeface_ref));
  }
};

}  // namespace

std::shared_ptr<ttoffice::tttext::IFontManager> CreateTextraFontManager() {
  return std::make_shared<TTAnimaXWebFontManager>();
}

}  // namespace animax
}  // namespace lynx
