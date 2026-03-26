// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/resource/asset/font_asset.h"

#include <cfloat>
#include <cmath>

#include "gtest/gtest.h"
#include "include/resource/raw_data.h"
#include "src/render/font.h"
#include "src/resource/asset/font_asset_model.h"

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
}

TEST(FontAssetTest, SetRawData) {
  auto font_asset = FontAsset{kFontAssetModelUrl};
  auto raw_data = RawData::MakeRawData(
      nullptr, 0, [](auto) {}, nullptr);
  font_asset.SetRawData(std::move(raw_data));
  EXPECT_TRUE(font_asset.IsLoaded());
  font_asset.GetFont();
}
