// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/font_asset_manager_textra.h"

#include <iostream>
#include <string>

#include "skity/text/typeface.hpp"
#include "src/layer/textra/textra_font_manager_factory.h"
#include "src/layer/textra/textra_include.h"
#include "src/render/font.h"

namespace lynx {
namespace animax {

class TTTextAssetFontManager : public ttoffice::tttext::IFontManager {
 public:
  TTTextAssetFontManager(const FontAssetMap &font_asset_map,
                         const std::string &asset_font_name_suffix)
      : font_asset_map_(font_asset_map),
        asset_font_name_suffix_(asset_font_name_suffix) {}
  int countFamilies() const override { return (int)font_asset_map_.size(); }
  TypefaceRef matchFamilyStyle(
      const char *family_name,
      const ttoffice::tttext::FontStyle &style) override {
    return legacyMakeTypeface(family_name, style);
  }
  TypefaceRef matchFamilyStyleCharacter(
      const char *family_name, const ttoffice::tttext::FontStyle &style,
      const char **bcp47, int bcp47_count, uint32_t character) override {
    return legacyMakeTypeface(family_name, style);
  }
  TypefaceRef makeFromFile(const char *path, int ttcIndex) override {
    return nullptr;
  }
  TypefaceRef legacyMakeTypeface(
      const char *family_name,
      ttoffice::tttext::FontStyle style) const override {
    if (family_name == nullptr || *family_name == 0) {
      return nullptr;
    }

    Font *font = FindFontIgnoreSuffix(family_name);
    if (font == nullptr) {
      return nullptr;
    }

    auto &skity_font = font->GetFont();
    auto type_face = skity_font.GetTypeface();
    if (type_face == nullptr) {
      return nullptr;
    }

    return std::make_shared<skity::textlayout::SkityTypefaceHelper>(type_face);
  }

 private:
  Font *FindFontIgnoreSuffix(const std::string &font_name) const {
    std::string base_name = font_name;
    if (font_name.size() > asset_font_name_suffix_.size() &&
        font_name.compare(font_name.size() - asset_font_name_suffix_.size(),
                          asset_font_name_suffix_.size(),
                          asset_font_name_suffix_) == 0) {
      base_name = font_name.substr(
          0, font_name.size() - asset_font_name_suffix_.size());
    }
    auto it = font_asset_map_.find(base_name);
    if (it != font_asset_map_.end()) {
      return it->second->GetFont();
    }
    return nullptr;
  }
  const FontAssetMap &font_asset_map_;
  std::string asset_font_name_suffix_;
};

FontAssetManagerTextra::FontAssetManagerTextra()
    : FontAssetManager(), font_mgr_collection_(nullptr, [](void *p) {
        delete static_cast<TTFontMgrCollection *>(p);
      }) {
  static int counter = 0;
  static const std::string prefix = "@_@";
  asset_font_name_suffix_ = prefix + std::to_string(counter++);
}

FontAssetManagerTextra::~FontAssetManagerTextra() = default;

std::string FontAssetManagerTextra::RedirectNameWithSuffix(
    const std::string &font_name) const {
  if (font_asset_map_.find(font_name) != font_asset_map_.end()) {
    return font_name + asset_font_name_suffix_;
  }
  return font_name;
}

void *FontAssetManagerTextra::GetFontMgrCollection() {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  if (font_mgr_collection_ != nullptr) {
    return font_mgr_collection_.get();
  }
  auto *collection = new TTFontMgrCollection(CreateTextraFontManager());
  collection->SetAssetFontManager(std::make_shared<TTTextAssetFontManager>(
      font_asset_map_, asset_font_name_suffix_));
  font_mgr_collection_.reset(collection);
  return font_mgr_collection_.get();
}

}  // namespace animax
}  // namespace lynx
