// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/resource/asset/video_asset.h"

#include <cfloat>
#include <cmath>

#include "gtest/gtest.h"
#include "src/resource/asset/video_asset_model.h"

using namespace lynx::animax;

namespace {

const std::array<int32_t, 4> kRgbFrame = {0, 0, 1920, 1080};
const std::array<int32_t, 4> kAFrame = {0, 0, 1920, 1080};
const std::string kId = "video_1";
const std::string kFileName = "example_video.mp4";
const std::string kDirName = "videos/";
const std::string kURL = "videos/example_video.mp4";

bool approximatelyEqual(float a, float b, float epsilon) {
  return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

class MockVideoAsset : public VideoAsset {
 public:
  using VideoAsset::VideoAsset;  // Inherit constructors
  static std::shared_ptr<VideoAsset> Make(VideoAssetModel model) {
    return std::shared_ptr<VideoAsset>(new MockVideoAsset(std::move(model)));
  }
  bool PrepareFrameData(const std::string& video_path) override {
    video_height_ = 1080;
    video_width_ = 1920;
    frame_rate_ = 24.0f;
    is_valid_ = true;
    return video_path == "videos/example_video.mp4";
  }
  int32_t GetFrameCount() const override { return 240; }
};

std::shared_ptr<VideoAsset> CreateTestVideoAsset() {
  return MockVideoAsset::Make(
      VideoAssetModel{kRgbFrame, kAFrame, kId, kDirName, kFileName});
}

}  // namespace

TEST(VideoAssetTest, Make) {
  auto video_asset = CreateTestVideoAsset();
  ASSERT_NE(nullptr, video_asset);
  EXPECT_EQ(1920, video_asset->GetRgbWidth());
  EXPECT_EQ(1080, video_asset->GetRgbHeight());
  EXPECT_EQ(kId, video_asset->Model().id);
  EXPECT_FALSE(video_asset->IsValid());
}

TEST(VideoAssetTest, LoadLocal) {
  auto video_asset = CreateTestVideoAsset();
  video_asset->LoadLocal(kURL);
  // Assume LoadLocal sets validity
  EXPECT_TRUE(video_asset->IsValid());
}

TEST(VideoAssetTest, VideoDimensionsAndFrameRate) {
  auto video_asset = CreateTestVideoAsset();
  video_asset->LoadLocal(kURL);
  EXPECT_EQ(1920, video_asset->GetVideoWidth());
  EXPECT_EQ(1080, video_asset->GetVideoHeight());
  // Assume a default frame rate for testing purposes
  EXPECT_TRUE(
      approximatelyEqual(video_asset->GetFrameRate(), 24.0f, FLT_EPSILON));
}

TEST(VideoAssetTest, ResetModel) {
  auto video_asset = CreateTestVideoAsset();
  video_asset->LoadLocal("path/to/video.mp4");
  EXPECT_TRUE(video_asset->IsValid());
  // Resetting the model should invalidate the asset
  VideoAssetModel newModel = {
      .rgb_frame = {100, 100, 1280, 720},
      .a_frame = {100, 100, 1280, 720},
      .id = "new_video_2",
      .dir_name = "new_videos/",
      .file_name = "new_example_video.mp4",
  };
  video_asset->ResetModel(newModel);
  EXPECT_FALSE(video_asset->IsValid());
  EXPECT_EQ(100, video_asset->GetRgbX());
  EXPECT_EQ(100, video_asset->GetRgbY());
  EXPECT_EQ(1280, video_asset->GetRgbWidth());
  EXPECT_EQ(720, video_asset->GetRgbHeight());

  EXPECT_EQ(100, video_asset->GetAlphaX());
  EXPECT_EQ(100, video_asset->GetAlphaY());
  EXPECT_EQ(1280, video_asset->GetAlphaWidth());
  EXPECT_EQ(720, video_asset->GetAlphaHeight());
}

TEST(VideoAssetTest, GetPrevKeyFrame) {
  auto video_asset = CreateTestVideoAsset();
  auto ret = video_asset->GetPrevKeyFrame(0);
  EXPECT_EQ(-1, ret);
}

TEST(VideoAssetTest, GetVideoParameterSets) {
  auto video_asset = CreateTestVideoAsset();
  auto ret = video_asset->GetVideoParameterSets();
  EXPECT_TRUE(ret.empty());
}

TEST(VideoAssetTest, GetFrameRawData) {
  auto video_asset = CreateTestVideoAsset();
  auto ret = video_asset->GetFrameRawData(0);
  EXPECT_TRUE(ret.empty());
}
