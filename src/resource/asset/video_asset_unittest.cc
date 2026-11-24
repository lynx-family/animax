// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/resource/asset/video_asset.h"

#include <cfloat>
#include <cmath>
#include <limits>

#include "gtest/gtest.h"
#include "include/resource/raw_data.h"
#include "src/model/composition_model.h"
#include "src/parser/alpha_player_asset/alpha_player_asset_parser.h"
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
  static std::shared_ptr<MockVideoAsset> Make(VideoAssetModel model) {
    return std::shared_ptr<MockVideoAsset>(
        new MockVideoAsset(std::move(model)));
  }
  bool PrepareFrameData(const std::string& video_path) override {
    SetPreparedMetadata(1920, 1080, 24.f, 240);
    return video_path == "videos/example_video.mp4";
  }
  int32_t GetFrameCount() const override { return frame_count_; }
  void SetPreparedMetadata(int32_t width, int32_t height, float frame_rate,
                           int32_t frame_count) {
    video_width_ = width;
    video_height_ = height;
    frame_rate_ = frame_rate;
    frame_count_ = frame_count;
    is_valid_ = true;
  }

 private:
  int32_t frame_count_ = 240;
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

TEST(VideoAssetTest, PrepareFrameData) {
  auto video_asset = CreateTestVideoAsset();
  auto raw_data = RawData::MakeRawData(nullptr, 0, nullptr, nullptr);
  auto ret = video_asset->PrepareFrameData(std::move(raw_data));
  EXPECT_FALSE(ret);
}

TEST(VideoAssetTest, IndependentPreparationRequiresBackendSupport) {
  auto asset = CreateTestVideoAsset();
  EXPECT_EQ(kNoSuchLoader, asset->PrepareFrameDataForPreload(kURL).code);
  EXPECT_FALSE(asset->IsValid());
}

TEST(VideoAssetTest, ResolvesMissingRegionsWithoutChangingThePreparedAsset) {
  auto asset = CreateTestVideoAsset();
  asset->LoadLocal(kURL);
  VideoAssetModel model{};
  ASSERT_EQ(kSuccess, asset->ResolvePreparedModel(&model).code);
  EXPECT_EQ((std::array<int32_t, 4>{0, 0, 960, 1080}), model.a_frame);
  EXPECT_EQ((std::array<int32_t, 4>{960, 0, 960, 1080}), model.rgb_frame);
  EXPECT_EQ(kAFrame, asset->Model().a_frame);
  EXPECT_EQ(kRgbFrame, asset->Model().rgb_frame);
}

TEST(VideoAssetTest, BuildsSilentAlphaPlayerTimelineFromPreparedVideo) {
  auto video = CreateTestVideoAsset();
  video->LoadLocal(kURL);
  VideoAssetModel slot{};
  slot.id = "0";
  ASSERT_EQ(kSuccess, video->ResolvePreparedModel(&slot).code);
  AlphaPlayerConfig config{.video_path = "original.mp4"};
  auto model = AlphaPlayerAssetParser::BuildFromPreparedVideo(
      config, video, slot, nullptr, 0.5f, false);
  ASSERT_NE(nullptr, model);
  EXPECT_EQ(video, model->GetVideos().at("0"));
  EXPECT_FLOAT_EQ(480.f, model->GetBounds().GetWidth());
  EXPECT_FLOAT_EQ(24.f, model->GetFrameRate());
  EXPECT_FLOAT_EQ(239.99f, model->GetEndFrame());
  EXPECT_TRUE(model->GetAudios().empty());
  EXPECT_EQ(1u, model->GetLayers().size());
  slot.id = "unknown";
  EXPECT_EQ(nullptr, AlphaPlayerAssetParser::BuildFromPreparedVideo(
                         config, video, slot, nullptr, 1.f, false));
}

TEST(VideoAssetTest, RebindingPreservesPreparedMediaAndExplicitRegions) {
  auto asset = MockVideoAsset::Make({});
  asset->SetPreparedMetadata(65, 48, 24.f, 12);
  VideoAssetModel model{
      {2, 0, 63, 24}, {0, 24, 65, 24}, kId, kDirName, std::string(80, 'v')};
  ASSERT_EQ(kSuccess, asset->SetPreparedModel(model).code);
  EXPECT_EQ(model.rgb_frame, asset->Model().rgb_frame);
  EXPECT_EQ(model.a_frame, asset->Model().a_frame);
  EXPECT_EQ(model.id, asset->Model().id);
  EXPECT_EQ(model.file_name, asset->Model().file_name);
  EXPECT_TRUE(asset->IsValid());
  EXPECT_EQ(65, asset->GetVideoWidth());
  EXPECT_EQ(48, asset->GetVideoHeight());
  EXPECT_FLOAT_EQ(24.f, asset->GetFrameRate());
  EXPECT_EQ(12, asset->GetFrameCount());
  const auto& stored = asset->Model();
  EXPECT_EQ(sizeof(VideoAsset) + stored.id.capacity() +
                stored.dir_name.capacity() + stored.file_name.capacity(),
            asset->GetMemoryUsageBytes());
}

TEST(VideoAssetTest, RejectsInvalidPreparedMetadataAndOddWidthRecovery) {
  auto asset = MockVideoAsset::Make({});
  VideoAssetModel model{};
  EXPECT_EQ(kInvalidRawData, asset->ResolvePreparedModel(&model).code);
  asset->SetPreparedMetadata(64, 48, 24.f, 12);
  EXPECT_EQ(kInvalidRawData, asset->ResolvePreparedModel(nullptr).code);

  struct Metadata {
    int32_t width;
    int32_t height;
    float frame_rate;
    int32_t frame_count;
  };
  const Metadata invalid[] = {
      {0, 48, 24.f, 12},
      {64, 0, 24.f, 12},
      {64, 48, std::numeric_limits<float>::quiet_NaN(), 12},
      {64, 48, 0.f, 12},
      {64, 48, 24.f, 0},
      {65, 48, 24.f, 12},
  };
  for (const auto& data : invalid) {
    asset->SetPreparedMetadata(data.width, data.height, data.frame_rate,
                               data.frame_count);
    EXPECT_EQ(kInvalidRawData, asset->ResolvePreparedModel(&model).code);
    EXPECT_EQ((std::array<int32_t, 4>{}), model.rgb_frame);
    EXPECT_EQ((std::array<int32_t, 4>{}), model.a_frame);
  }
}

TEST(VideoAssetTest, RejectsInvalidRegionsWithoutReplacingThePreparedModel) {
  auto asset = CreateTestVideoAsset();
  asset->LoadLocal(kURL);
  const std::array<int32_t, 4> invalid_regions[] = {
      {},
      {-1, 0, 1, 1},
      {0, -1, 1, 1},
      {0, 0, 0, 1},
      {0, 0, 1, 0},
      {1, 0, std::numeric_limits<int32_t>::max(), 1},
      {0, 1, 1, 1080},
  };
  for (const auto& region : invalid_regions) {
    auto model = asset->Model();
    model.rgb_frame = region;
    EXPECT_EQ(kInvalidRawData, asset->ResolvePreparedModel(&model).code);
    EXPECT_EQ(region, model.rgb_frame);
    EXPECT_EQ(kAFrame, model.a_frame);
  }
  auto model = asset->Model();
  model.a_frame = {};
  model.id = "replacement";
  EXPECT_EQ(kInvalidRawData, asset->SetPreparedModel(model).code);
  EXPECT_EQ(kId, asset->Model().id);
  EXPECT_EQ(kAFrame, asset->Model().a_frame);
  EXPECT_TRUE(asset->IsValid());
}
