// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/layer/textra/font_asset_manager_textra.h"

#include "gtest/gtest.h"
#include "skity/text/font.hpp"
#include "skity/text/typeface.hpp"
#include "src/base/thread/thread_assert.h"
#include "src/layer/textra/textra_include.h"
#include "src/render/font.h"
#include "src/testing/font_test_data.h"

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
  font_asset->SetRawData(MakeTestFontData());
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
  ASSERT_NE(nullptr, typeface_model_match_0);
  ASSERT_NE(nullptr, typeface_model_match_1);
  EXPECT_EQ(typeface_model_match_0->GetUniqueId(),
            typeface_model_match_1->GetUniqueId());

  FontStyle style_2 = ttoffice::tttext::FontStyle(
      FontStyle::kLight_Weight, FontStyle::kSemiCondensed_Width,
      FontStyle::kOblique_Slant);
  TypefaceRef typeface_model_match_2 = asset_font_manager->legacyMakeTypeface(
      kFontAssetModel.name.c_str(), style_2);
  ASSERT_NE(nullptr, typeface_model_match_2);
  EXPECT_EQ(typeface_model_match_0->GetUniqueId(),
            typeface_model_match_2->GetUniqueId());
}

TEST(FontAssetManagerTest, LateRemoteFontDoesNotReuseFallbackDescriptor) {
  ThreadAssert::Init(ThreadAssert::Type::kGPU);
  auto model = kFontAssetModel;
  model.path_origin = FontPathOrigin::kScriptUrl;
  model.path = "https://example.com/font.ttf";
  auto asset = std::make_shared<FontAsset>(model);
  FontAssetManagerTextra manager;
  manager.GetFontAssetMap().emplace(model.name, asset);
  auto *collection =
      static_cast<TTFontMgrCollection *>(manager.GetFontMgrCollection());
  // Provide a deterministic fallback even when the host has no system fonts.
  FontAssetManagerTextra fallback_manager;
  auto fallback_model = kFontAssetModel;
  fallback_model.name = "TestFallbackFont";
  auto fallback_asset = std::make_shared<FontAsset>(fallback_model);
  fallback_asset->SetRawData(MakeTestFontData(TestFontVariant::kWide));
  fallback_manager.GetFontAssetMap().emplace(fallback_model.name,
                                             fallback_asset);
  auto *fallback_collection = static_cast<TTFontMgrCollection *>(
      fallback_manager.GetFontMgrCollection());
  collection->SetTestFontManager(fallback_collection->GetAssetFontManager());
  collection->SetDefaultFontManager(nullptr);
  collection->SetDefaultSystemFontFamily(fallback_model.name);
  FontDescriptor descriptor;
  descriptor.font_family_list_.push_back(
      manager.RedirectNameWithSuffix(model.name));
  auto fallback_faces = collection->findTypefaces(descriptor);
  ASSERT_FALSE(fallback_faces.empty());
  EXPECT_FALSE(asset->IsLoaded());

  asset->SetRawData(MakeTestFontData());

  auto updated_name = manager.RedirectNameWithSuffix(model.name);
  EXPECT_NE(descriptor.font_family_list_.front(), updated_name);
  descriptor.font_family_list_ = {updated_name};
  auto loaded_faces = collection->findTypefaces(descriptor);
  ASSERT_EQ(1u, loaded_faces.size());
  EXPECT_EQ(asset->GetFont()->GetFont().GetTypeface()->TypefaceId(),
            loaded_faces.front()->GetUniqueId());
  EXPECT_NE(fallback_faces.front()->GetUniqueId(),
            loaded_faces.front()->GetUniqueId());

  // A shaped result may outlive the asset data that produced its typeface.
  asset->ResetModel(model);
  EXPECT_NE(0u, loaded_faces.front()->UnicharToGlyph('A'));
  EXPECT_GT(loaded_faces.front()->GetHorizontalAdvance(
                loaded_faces.front()->UnicharToGlyph('A'), 24.f),
            0.f);
}

#if defined(OS_WIN)
TEST(FontAssetManagerTest, TextraFallsBackWhenLottieFontIsUnavailable) {
  ThreadAssert::Init(ThreadAssert::Type::kGPU);

  FontAssetManagerTextra font_asset_manager;
  auto *font_mgr_collection = static_cast<TTFontMgrCollection *>(
      font_asset_manager.GetFontMgrCollection());
  FontDescriptor descriptor;
  descriptor.font_family_list_.push_back("__missing_animax_font__");

  EXPECT_FALSE(font_mgr_collection->findTypefaces(descriptor).empty());
}
#endif
