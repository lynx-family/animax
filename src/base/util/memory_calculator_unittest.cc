// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/memory_calculator.h"

#include "gtest/gtest.h"
#include "src/model/composition_model.h"
#include "src/model/rect_model.h"
#include "src/resource/asset/image_asset.h"
#include "src/resource/asset/video_asset.h"

using namespace lynx::animax;

class MemoryCalculatorTest : public ::testing::Test {
 protected:
  MemoryCalculatorTest() = default;
  ~MemoryCalculatorTest() override = default;

  std::shared_ptr<CompositionModel> CreateCompositionModel(int32_t width,
                                                           int32_t height) {
    auto model = std::make_shared<CompositionModel>(1.0f);
    auto bounds = std::make_unique<RectF>(0, 0, static_cast<float>(width),
                                          static_cast<float>(height));
    model->Init(std::move(bounds), 0.0f, 100.0f, 30.0f, false);
    return model;
  }

  std::shared_ptr<ImageAsset> CreateImageAsset(int32_t width, int32_t height,
                                               const std::string& id) {
    ImageAssetModel model;
    model.width = width;
    model.height = height;
    model.id = id;
    return std::make_shared<ImageAsset>(model);
  }
};

class MockVideoAssetModel : public VideoAsset {
 public:
  explicit MockVideoAssetModel() : VideoAsset(VideoAssetModel()) {
    PrepareFrameData("");
  }
  bool PrepareFrameData(const std::string& video_path) override {
    video_width_ = 100;
    video_height_ = 100;
    frame_rate_ = 30.0f;
    return true;
  }

  int32_t GetFrameCount() const override { return 10; }
};

// Test memory calculation with one image
TEST_F(MemoryCalculatorTest, CalculateMemoryUsageWithOneImage) {
  auto model = CreateCompositionModel(100, 100);
  auto image_asset = CreateImageAsset(200, 150, "image_0");
  model->GetImages()["image_0"] = image_asset;

  MemoryCalculatorParams params;
  params.model = model;
  params.view_width = 100;
  params.view_height = 100;
  params.scale_factor_x = 1.0f;
  params.scale_factor_y = 1.0f;
  MemoryUsageInfo info = MemoryCalculator::CalculateMemoryUsage(params);

  EXPECT_GT(info.image_memory, 0);
  EXPECT_EQ(info.video_texture_memory, 0);
  EXPECT_EQ(info.video_cache_memory, 0);
  EXPECT_GT(info.total_memory, 0);
}

// Test memory calculation with one video
TEST_F(MemoryCalculatorTest, CalculateMemoryUsageWithOneVideo) {
  auto model = CreateCompositionModel(100, 100);
  auto video_asset = std::make_shared<MockVideoAssetModel>();
  model->GetVideos()["video_0"] = video_asset;

  MemoryCalculatorParams params;
  params.model = model;
  params.view_width = 100;
  params.view_height = 100;
  params.scale_factor_x = 1.0f;
  params.scale_factor_y = 1.0f;
  MemoryUsageInfo info = MemoryCalculator::CalculateMemoryUsage(params);

  EXPECT_EQ(info.image_memory, 0);
  EXPECT_GT(info.video_cache_memory, 0);
  EXPECT_GT(info.total_memory, 0);

  info.image_memory = 100;
  info.video_texture_memory = 100;
  info.video_cache_memory = 100;
  info.mask_matte_memory = 100;
  info.screen_buffer_memory = 100;
  info.total_memory = 500;
  auto description = info.GetDescription();
  EXPECT_EQ(description.empty(), false);
}

// Test memory calculation with only width and height
TEST_F(MemoryCalculatorTest, CalculateMemoryUsageWithOnlyWidthAndHeight) {
  auto model = CreateCompositionModel(100, 100);

  MemoryCalculatorParams params;
  params.model = model;
  params.view_width = 100;
  params.view_height = 100;
  params.scale_factor_x = 1.0f;
  params.scale_factor_y = 1.0f;
  MemoryUsageInfo info = MemoryCalculator::CalculateMemoryUsage(params);

  EXPECT_EQ(info.image_memory, 0);
  EXPECT_EQ(info.video_texture_memory, 0);
  EXPECT_EQ(info.video_cache_memory, 0);
  EXPECT_GT(info.screen_buffer_memory, 0);
  EXPECT_GT(info.total_memory, 0);
}
