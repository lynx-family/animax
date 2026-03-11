// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/layer/textra/font_asset_manager_textra.h"

#include "gtest/gtest.h"
#include "skity/text/typeface.hpp"
#include "src/base/thread/thread_assert.h"
#include "src/layer/textra/textra_include.h"
#include "src/render/font.h"

using namespace lynx::animax;
using namespace ttoffice::tttext;

namespace {
const auto kFontAssetModel = FontAssetModel{.family = "Heiti SC",
                                            .name = "STHeitiSC-Light",
                                            .style = "Light",
                                            .ascent = 73.5989987850189};
}  // namespace

TEST(FontAssetManagerTest, Default) {
  ThreadAssert::Init(ThreadAssert::Type::kGPU);

  FontAssetManager font_asset_manager;
  EXPECT_EQ(nullptr, font_asset_manager.GetFontMgrCollection());

  FontAssetMap &font_asset_map = font_asset_manager.GetFontAssetMap();
  EXPECT_EQ(0, font_asset_map.size());

  auto font_asset = std::make_shared<FontAsset>(kFontAssetModel);
  font_asset_map.insert(std::make_pair(kFontAssetModel.name, font_asset));
  EXPECT_EQ(1, font_asset_map.size());

  auto font_asset_result = font_asset_map[kFontAssetModel.name];
  EXPECT_NE(nullptr, font_asset_result);
  EXPECT_EQ(font_asset_result.get(), font_asset.get());

  auto redirected_font_name =
      font_asset_manager.RedirectNameWithSuffix(kFontAssetModel.name);
  EXPECT_EQ(redirected_font_name, kFontAssetModel.name);

  std::string name_not_exit = "abc_def";
  auto redirected_not_exist =
      font_asset_manager.RedirectNameWithSuffix(name_not_exit);
  EXPECT_EQ(redirected_not_exist, name_not_exit);
}

TEST(FontAssetManagerTest, Textra) {
  ThreadAssert::Init(ThreadAssert::Type::kGPU);

  FontAssetManagerTextra font_asset_manager;
  TTFontMgrCollection *font_mgr_collection = static_cast<TTFontMgrCollection *>(
      font_asset_manager.GetFontMgrCollection());
  EXPECT_NE(nullptr, font_mgr_collection);
  auto default_font_manager = font_mgr_collection->GetDefaultFontManager();
  EXPECT_NE(nullptr, default_font_manager);

  auto asset_font_manager = font_mgr_collection->GetAssetFontManager();
  EXPECT_NE(nullptr, asset_font_manager);

  FontAssetMap &font_asset_map = font_asset_manager.GetFontAssetMap();
  EXPECT_EQ(0, font_asset_map.size());

  auto font_asset = std::make_shared<FontAsset>(kFontAssetModel);
  font_asset_map.insert(std::make_pair(kFontAssetModel.name, font_asset));
  EXPECT_EQ(1, font_asset_map.size());

  auto font_asset_result = font_asset_map[kFontAssetModel.name];
  EXPECT_NE(nullptr, font_asset_result);
  EXPECT_EQ(font_asset_result.get(), font_asset.get());

  auto redirected_font_name =
      font_asset_manager.RedirectNameWithSuffix(kFontAssetModel.name);
  EXPECT_NE(redirected_font_name, kFontAssetModel.name);  // with suffix

  std::string name_not_exit = "abc_def";
  auto redirected_not_exist =
      font_asset_manager.RedirectNameWithSuffix(name_not_exit);
  EXPECT_EQ(redirected_not_exist, name_not_exit);  // without suffix

  auto *skity_font = font_asset->GetFont();
  EXPECT_NE(nullptr, skity_font);
  EXPECT_TRUE(font_asset->IsLoaded());

  FontStyle style_default = FontStyle{};
  TypefaceRef typeface_null =
      asset_font_manager->legacyMakeTypeface(nullptr, style_default);
  EXPECT_EQ(nullptr, typeface_null);
  TypefaceRef typeface_empty =
      asset_font_manager->legacyMakeTypeface("", style_default);
  EXPECT_EQ(nullptr, typeface_empty);
  TypefaceRef typeface_not_exists =
      asset_font_manager->legacyMakeTypeface("Songti SC", style_default);
  EXPECT_EQ(nullptr, typeface_not_exists);

  TypefaceRef typeface_model_match_0 = asset_font_manager->matchFamilyStyle(
      kFontAssetModel.name.c_str(), style_default);
  FontStyle style_1 = ttoffice::tttext::FontStyle(
      FontStyle::kNormal_Weight, FontStyle::kSemiExpanded_Width,
      FontStyle::kItalic_Slant);
  TypefaceRef typeface_model_match_1 =
      asset_font_manager->matchFamilyStyleCharacter(
          kFontAssetModel.name.c_str(), style_1, nullptr, 0, 0);
  EXPECT_EQ(typeface_model_match_0, typeface_model_match_1);

  FontStyle style_2 = ttoffice::tttext::FontStyle(
      FontStyle::kLight_Weight, FontStyle::kSemiCondensed_Width,
      FontStyle::kOblique_Slant);
  TypefaceRef typeface_model_match_2 = asset_font_manager->legacyMakeTypeface(
      kFontAssetModel.name.c_str(), style_2);
  EXPECT_EQ(typeface_model_match_0, typeface_model_match_2);
}
