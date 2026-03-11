// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/resource/asset/image_asset.h"

#include <GL/osmesa.h>

#include "gtest/gtest.h"
#include "include/resource/bitmap.h"
#include "src/resource/asset/image_asset_model.h"

using namespace lynx::animax;

namespace {

const auto kImageAssetModel = ImageAssetModel{.width = 132,
                                              .height = 745,
                                              .id = "image_0",
                                              .dir_name = "images/",
                                              .file_name = "img_0.png"};

const auto kImageAssetModelPolyfill = ImageAssetModel{
    .width = 132,
    .height = 745,
    .id = "image_0",
    .dir_name = "images/",
    .file_name = "%s"  // Using src-polyfill
};

bool ExpectImageAssetModelEqual(const ImageAssetModel& lhs,
                                const ImageAssetModel& rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height &&
         lhs.id == rhs.id && lhs.dir_name == rhs.dir_name &&
         lhs.file_name == rhs.file_name;
}

}  // namespace

TEST(ImageAssetTest, DefaultConstructor) {
  ImageAsset image_asset{kImageAssetModel};
  EXPECT_FALSE(image_asset.IsLoaded());
  EXPECT_FALSE(image_asset.IsBase64());
  EXPECT_TRUE(
      ExpectImageAssetModelEqual(kImageAssetModel, image_asset.Model()));
}

TEST(ImageAssetTest, ConstructedFromAssetModel) {
  ImageAsset image_asset{kImageAssetModel};
  EXPECT_FALSE(image_asset.IsLoaded());
  auto bitmap = Bitmap::MakeRGBA(
      0, 0, nullptr, [](auto) {}, nullptr);
  image_asset.SetBitmap(std::move(bitmap));
  EXPECT_TRUE(image_asset.IsLoaded());
  auto* image = image_asset.GetImage(nullptr);
  EXPECT_NE(nullptr, image);
}

TEST(ImageAssetTest, PolyfillHandling) {
  ImageAsset image_asset{kImageAssetModelPolyfill};
  EXPECT_TRUE(image_asset.IsPolyfill());
  EXPECT_EQ("%s", image_asset.Model().file_name);
}

TEST(ImageAssetTest, ResetModel) {
  ImageAsset image_asset{kImageAssetModel};
  auto bitmap = Bitmap::MakeRGBA(
      0, 0, nullptr, [](auto) {}, nullptr);
  image_asset.SetBitmap(std::move(bitmap));
  EXPECT_TRUE(image_asset.IsLoaded());
  image_asset.ResetModel(kImageAssetModelPolyfill);
  EXPECT_FALSE(image_asset.IsLoaded());
  EXPECT_TRUE(image_asset.IsPolyfill());
}
