// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/preloaded_asset_loader.h"

#include <limits>

#include "gtest/gtest.h"
#include "src/base/thread/task_runner.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/testing/font_test_data.h"
#include "testing/gtest_expectation.h"

using namespace lynx::animax;

namespace {

PreloadedAssetResponse Prepare(ResourceType type, ResourcePayload payload,
                               ResourceRequest request = {}) {
  auto loader = MakeLambdaLoader<ResourceRequest, ResourceResponse>(
      [payload = std::move(payload)](ResourceRequest request,
                                     auto callback) mutable {
        EXPECT_TRUE(request.cache_by_content);
        callback({.payload = std::move(payload)}, {});
      });
  auto done = GTestExpectation::Make();
  auto result = std::make_shared<PreloadedAssetResponse>();
  LoadPreloadedAsset(
      type, std::move(request), std::move(loader),
      [result, done](PreloadedAssetResponse response) {
        EXPECT_TRUE(GetAnimaXResourceThread()->RunsTasksOnCurrentThread());
        *result = std::move(response);
        done->Fulfill();
      });
  if (done->Wait()) {
    ADD_FAILURE() << "Resource preparation did not complete";
    return {};
  }
  return std::move(*result);
}

TEST(PreloadedAssetLoaderTest, ValidatesImageDimensionsAndMemorySize) {
  static constexpr uint32_t pixel = 0;
  auto bitmap = [](uint32_t width, uint32_t height) {
    return MakeBitmapResourcePayload(
        Bitmap::MakeRGBA(width, height, &pixel, nullptr, nullptr));
  };
  ResourceRequest exact{.width = 1,
                        .height = 1,
                        .id = "icon",
                        .image_size_mode = ImageSizeMode::kExact};
  auto image = Prepare(ResourceType::kImage, bitmap(1, 1), exact);
  EXPECT_EQ(kSuccess, image.result.error.code);
  EXPECT_EQ("icon", image.result.id);
  EXPECT_EQ(ResourceType::kImage, image.result.type);
  ASSERT_NE(nullptr, image.bitmap);
  EXPECT_EQ(&pixel, image.bitmap->Pixels());
  EXPECT_EQ(sizeof(Bitmap) + sizeof(pixel), image.payload_bytes);

  auto wrong_size = Prepare(
      ResourceType::kImage, bitmap(1, 1),
      {.width = 2, .height = 1, .image_size_mode = ImageSizeMode::kExact});
  EXPECT_EQ(kInvalidImageData, wrong_size.result.error.code);
  EXPECT_EQ(nullptr, wrong_size.bitmap);
  EXPECT_EQ(0, wrong_size.payload_bytes);
  EXPECT_EQ(kInvalidImageData,
            Prepare(ResourceType::kImage, {}).result.error.code);
  auto max = std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(kInvalidImageData,
            Prepare(ResourceType::kImage, bitmap(max, max)).result.error.code);
}

TEST(PreloadedAssetLoaderTest, PublishesOnlyValidFontData) {
  auto data = MakeTestFontData();
  auto length = data->Length();
  auto font =
      Prepare(ResourceType::kFont, MakeRawDataResourcePayload(std::move(data)));
  ASSERT_NE(nullptr, font.font_data);
  EXPECT_EQ(kSuccess, font.result.error.code);
  EXPECT_EQ(sizeof(RawData) + length, font.payload_bytes);

  static constexpr uint8_t invalid[] = {1, 2, 3};
  auto corrupt = Prepare(ResourceType::kFont,
                         MakeRawDataResourcePayload(RawData::MakeRawData(
                             invalid, sizeof(invalid), nullptr, nullptr)));
  EXPECT_EQ(PreloadedAssetStage::kPrepare, corrupt.stage);
  EXPECT_EQ(kInvalidRawData, corrupt.result.error.code);
  EXPECT_EQ(nullptr, corrupt.font_data);
  EXPECT_EQ(0, corrupt.payload_bytes);
  EXPECT_EQ(kInvalidRawData,
            Prepare(ResourceType::kFont, {}).result.error.code);
}

TEST(PreloadedAssetLoaderTest, InvalidMediaPayloadsDoNotPublishAnAsset) {
  for (auto type : {ResourceType::kAudio, ResourceType::kVideo}) {
    auto media = Prepare(type, MakeRawDataResourcePayload(nullptr));
    EXPECT_NE(kSuccess, media.result.error.code);
    EXPECT_EQ(PreloadedAssetStage::kPrepare, media.stage);
    EXPECT_EQ(0, media.payload_bytes);
    EXPECT_EQ(nullptr, media.audio);
    EXPECT_EQ(nullptr, media.video);
  }
  EXPECT_EQ(kInvalidArgument,
            Prepare(ResourceType::kUnknown, {}).result.error.code);
}

TEST(PreloadedAssetLoaderTest, MissingLoaderFailsAtLoadStage) {
  auto done = GTestExpectation::Make();
  LoadPreloadedAsset(ResourceType::kFont, {.id = "font"}, nullptr,
                     [done](auto response) {
                       EXPECT_EQ("font", response.result.id);
                       EXPECT_EQ(kInvalidLoader, response.result.error.code);
                       EXPECT_EQ(PreloadedAssetStage::kLoad, response.stage);
                       done->Fulfill();
                     });
  EXPECT_FALSE(done->Wait());
}

TEST(PreloadedAssetLoaderTest, BatchKeepsRequestOrderAndFirstCompletion) {
  std::vector<Loader<ResourceRequest, ResourceResponse>::CallbackType> pending;
  auto loader = MakeLambdaLoader<ResourceRequest, ResourceResponse>(
      [&pending](ResourceRequest, auto callback) {
        pending.emplace_back(std::move(callback));
      });
  auto done = GTestExpectation::Make();
  auto results = std::make_shared<std::vector<PreloadedAssetResponse>>();
  LoadPreloadedAssets({{ResourceType::kFont, {.id = "font"}},
                       {ResourceType::kImage, {.id = "icon"}}},
                      loader, [done, results](auto responses) {
                        *results = std::move(responses);
                        done->Fulfill();
                      });
  ASSERT_EQ(2u, pending.size());
  pending[1]({}, {kBitmapError, "decode failed"});
  pending[1]({}, {kFileIOFailure, "second completion"});
  pending[0]({}, {kFileDoesNotExist, "missing font"});
  ASSERT_FALSE(done->Wait());
  ASSERT_EQ(2u, results->size());
  EXPECT_EQ("font", (*results)[0].result.id);
  EXPECT_EQ(kFileDoesNotExist, (*results)[0].result.error.code);
  EXPECT_EQ(PreloadedAssetStage::kLoad, (*results)[0].stage);
  EXPECT_EQ("icon", (*results)[1].result.id);
  EXPECT_EQ(kBitmapError, (*results)[1].result.error.code);
  EXPECT_EQ(PreloadedAssetStage::kPrepare, (*results)[1].stage);
}

TEST(PreloadedAssetLoaderTest, EmptyBatchCompletesOnResourceThread) {
  auto done = GTestExpectation::Make();
  LoadPreloadedAssets({}, nullptr, [done](auto responses) {
    EXPECT_TRUE(GetAnimaXResourceThread()->RunsTasksOnCurrentThread());
    EXPECT_TRUE(responses.empty());
    done->Fulfill();
  });
  EXPECT_FALSE(done->Wait());
}

}  // namespace
