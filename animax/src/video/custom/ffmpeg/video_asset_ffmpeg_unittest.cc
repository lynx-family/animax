// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/ffmpeg/video_asset_ffmpeg.h"

#include "gtest/gtest.h"
#include "src/resource/asset/video_asset.h"

using namespace lynx::animax;

namespace {

class VideoAssetFFmpegTest : public ::testing::Test {
 protected:
  VideoAssetFFmpegTest() = default;
  ~VideoAssetFFmpegTest() override = default;
};

TEST_F(VideoAssetFFmpegTest, Create) {
  auto asset = VideoAsset::Make(VideoAssetModel());
  EXPECT_NE(nullptr, asset.get());
}

TEST_F(VideoAssetFFmpegTest, InitialState) {
  auto asset = VideoAsset::Make(VideoAssetModel());
  EXPECT_EQ(0, asset->GetFrameCount());
  EXPECT_TRUE(asset->GetVideoParameterSets().empty());
  EXPECT_TRUE(asset->GetFrameRawData(0).empty());
  EXPECT_TRUE(asset->GetFrameRawData(-1).empty());
}

TEST_F(VideoAssetFFmpegTest, GetPrevKeyFrameEmpty) {
  auto asset = VideoAsset::Make(VideoAssetModel());
  EXPECT_EQ(0, asset->GetPrevKeyFrame(0));
  EXPECT_EQ(0, asset->GetPrevKeyFrame(10));
  EXPECT_EQ(0, asset->GetPrevKeyFrame(-1));
}

}  // namespace
