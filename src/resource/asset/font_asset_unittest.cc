// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/resource/asset/font_asset.h"

#include <cfloat>
#include <cmath>

#include "gtest/gtest.h"
#include "include/resource/raw_data.h"
#include "skity/text/font.hpp"
#include "skity/text/typeface.hpp"
#include "src/render/font.h"
#include "src/resource/asset/font_asset_model.h"
#include "src/testing/font_test_data.h"

using namespace lynx::animax;

namespace {

bool approximatelyEqual(float a, float b, float epsilon) {
  return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

const auto kFontAssetModel = FontAssetModel{.family = "Heiti SC",
                                            .name = "STHeitiSC-Light",
                                            .style = "Light",
                                            .ascent = 73.5989987850189};

const auto kFontAssetModelUrl = FontAssetModel{
    .family = "Heiti SC",
    .name = "STHeitiSC-Light",
    .style = "Light",
    .ascent = 73.5989987850189,
    .path_origin = FontPathOrigin::kScriptUrl,
};

bool ExpectFontAssetModelEqual(const FontAssetModel& lhs,
                               const FontAssetModel& rhs) {
  return lhs.family == rhs.family && lhs.name == rhs.name &&
         lhs.style == rhs.style && lhs.path == rhs.path &&
         lhs.path_origin == rhs.path_origin &&
         approximatelyEqual(lhs.ascent, rhs.ascent, FLT_EPSILON);
}
}  // namespace

TEST(FontAssetTest, FontAssetDefaultConstructor) {
  auto font_asset = FontAsset{};
  EXPECT_FALSE(font_asset.IsLoaded());
  EXPECT_TRUE(ExpectFontAssetModelEqual(FontAssetModel{}, font_asset.Model()));
}

TEST(FontAssetTest, FontAssetContructedFromAssetModel) {
  auto font_asset = FontAsset{kFontAssetModel};
  EXPECT_FALSE(font_asset.IsLoaded());
  auto* const font = font_asset.GetFont();
  EXPECT_NE(nullptr, font);
  EXPECT_TRUE(font_asset.IsLoaded());
  EXPECT_TRUE(ExpectFontAssetModelEqual(kFontAssetModel, font_asset.Model()));
  // The second call to GetFont should return the same font.
  EXPECT_EQ(font, font_asset.GetFont());
}

TEST(FontAssetTest, FontAssetResetModel) {
  auto font_asset = FontAsset{kFontAssetModel};
  auto* const font = font_asset.GetFont();
  // The second call to GetFont should return the same font.
  EXPECT_EQ(font, font_asset.GetFont());
  font_asset.ResetModel(kFontAssetModel);
  EXPECT_FALSE(font_asset.IsLoaded());
  EXPECT_EQ(1u, font_asset.GetDataRevision());
}

TEST(FontAssetTest, RemoteFallbackDoesNotMarkLoaded) {
  auto font_asset = FontAsset{kFontAssetModelUrl};
  auto* font = font_asset.GetFont();
  ASSERT_NE(nullptr, font);
  EXPECT_EQ(font, font_asset.GetFont());
  EXPECT_FALSE(font_asset.IsLoaded());
  EXPECT_EQ(0u, font_asset.GetDataRevision());
}

TEST(FontAssetTest, EmptyRawDataDoesNotMarkLoaded) {
  auto font_asset = FontAsset{kFontAssetModelUrl};
  font_asset.SetRawData(nullptr);
  EXPECT_FALSE(font_asset.IsLoaded());

  font_asset.SetRawData(RawData::MakeRawData(nullptr, 1, nullptr, nullptr));
  EXPECT_FALSE(font_asset.IsLoaded());

  const uint8_t byte = 1;
  font_asset.SetRawData(RawData::MakeRawData(&byte, 0, nullptr, nullptr));
  EXPECT_FALSE(font_asset.IsLoaded());
  EXPECT_EQ(3u, font_asset.GetDataRevision());
  EXPECT_NE(nullptr, font_asset.GetFont());
  EXPECT_FALSE(font_asset.IsLoaded());
}

TEST(FontAssetTest, RemoteFontRefreshesAfterDataArrives) {
  auto font_asset = FontAsset{kFontAssetModelUrl};
  auto* fallback = font_asset.GetFont();
  ASSERT_NE(nullptr, fallback);
  auto fallback_typeface = fallback->GetFont().GetTypeface();

  auto raw_data = MakeTestFontData();
  ASSERT_NE(nullptr, raw_data);
  ASSERT_GT(raw_data->Length(), 0u);
  font_asset.SetRawData(std::move(raw_data));
  EXPECT_TRUE(font_asset.IsLoaded());
  EXPECT_EQ(1u, font_asset.GetDataRevision());
  // Publishing bytes on the resource thread must not destroy the GPU's Font.
  EXPECT_EQ(fallback_typeface, fallback->GetFont().GetTypeface());

  auto* loaded = font_asset.GetFont();
  ASSERT_NE(nullptr, loaded);
  auto loaded_typeface = loaded->GetFont().GetTypeface();
  ASSERT_NE(nullptr, loaded_typeface);
  EXPECT_NE(fallback_typeface, loaded_typeface);
  EXPECT_GT(loaded_typeface->UnicharToGlyph('A'), 0u);
  EXPECT_EQ(loaded, font_asset.GetFont());

  font_asset.SetRawData(nullptr);
  EXPECT_FALSE(font_asset.IsLoaded());
  EXPECT_EQ(2u, font_asset.GetDataRevision());
  EXPECT_EQ(loaded_typeface, loaded->GetFont().GetTypeface());
  EXPECT_NE(loaded_typeface, font_asset.GetFont()->GetFont().GetTypeface());
  // A shape cache may retain this typeface after the asset releases its bytes.
  EXPECT_GT(loaded_typeface->UnicharToGlyph('Z'), 0u);
}

TEST(FontAssetTest, TypefaceRetainsBytesAfterAssetDestruction) {
  std::shared_ptr<skity::Typeface> typeface;
  {
    auto font_asset = FontAsset{kFontAssetModelUrl};
    auto raw_data = MakeTestFontData();
    ASSERT_NE(nullptr, raw_data);
    ASSERT_GT(raw_data->Length(), 0u);
    font_asset.SetRawData(std::move(raw_data));
    typeface = font_asset.GetFont()->GetFont().GetTypeface();
    ASSERT_NE(nullptr, typeface);
  }
  EXPECT_GT(typeface->UnicharToGlyph('Z'), 0u);
}

TEST(FontAssetTest, SnapshotRetainsFontAfterResourceRefresh) {
  auto font_asset = FontAsset{kFontAssetModelUrl};
  auto snapshot = font_asset.GetFontSnapshot();
  ASSERT_NE(nullptr, snapshot);
  std::weak_ptr<Font> previous_font = snapshot;

  auto raw_data = MakeTestFontData();
  ASSERT_NE(nullptr, raw_data);
  ASSERT_GT(raw_data->Length(), 0u);
  font_asset.SetRawData(std::move(raw_data));
  auto refreshed = font_asset.GetFontSnapshot();
  ASSERT_NE(nullptr, refreshed);
  EXPECT_NE(snapshot, refreshed);
  EXPECT_FALSE(previous_font.expired());
  EXPECT_EQ(refreshed.get(), font_asset.GetFont());

  snapshot.reset();
  EXPECT_TRUE(previous_font.expired());
}
