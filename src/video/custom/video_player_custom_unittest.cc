// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/video_player_custom.h"

#include "gtest/gtest.h"
#include "src/render/texture_info_gl.h"
#include "src/resource/asset/video_asset.h"
#include "src/video/custom/video_decoder_custom.h"

using namespace lynx::animax;

namespace {

class MockVideoDecoderCustom : public VideoDecoderCustom {
 public:
  bool IsValid() override { return true; }

  bool CreateDecoder() override { return true; }

  bool DestroyDecoder() override { return true; }

  std::shared_ptr<YUVFrameInfo> DecodeFrameData(
      std::vector<uint8_t>& data,
      const std::shared_ptr<YUVFrameInfo>& reusable_frame) override {
    return std::make_shared<YUVFrameInfo>();
  }

  std::unique_ptr<TextureInfo> UpdateTexturesFromYuvFrame(
      const std::shared_ptr<YUVFrameInfo>& frame_info) override {
    return std::make_unique<TextureInfoGL>(0, 0, 0, 0);
  }
};

class MockVideoAsset : public VideoAsset {
 public:
  explicit MockVideoAsset(VideoAssetModel model)
      : VideoAsset(std::move(model)){

        };
  ~MockVideoAsset() override = default;

  bool PrepareFrameData(const std::string& video_path) override { return true; }

  int32_t GetFrameCount() const override { return 20; }

  int GetPrevKeyFrame(const int32_t frame) const override { return frame; }

  std::vector<uint8_t> GetVideoParameterSets() const override {
    return {0, 0, 0, 0};
  }

  std::vector<uint8_t> GetFrameRawData(int32_t frame) const override {
    return {0, 0, 0, 0};
  }
};

class VideoPlayerCustomTest : public ::testing::Test {
 protected:
  VideoPlayerCustomTest() = default;
  ~VideoPlayerCustomTest() override = default;
};

TEST_F(VideoPlayerCustomTest, UpdateTexture) {
  auto decoder_ = std::make_unique<MockVideoDecoderCustom>();
  EXPECT_EQ(true, decoder_->IsValid());

  auto player_ = std::make_shared<VideoPlayerCustom>(std::move(decoder_));
  auto asset = std::make_shared<MockVideoAsset>(VideoAssetModel());
  player_->AttachAsset(asset);
  EXPECT_NE(nullptr, asset.get());
  EXPECT_EQ(20, asset->GetFrameCount());

  for (int i = 0; i < asset->GetFrameCount(); i++) {
    auto ret = player_->UpdateTexture(i);
    EXPECT_NE(nullptr, ret);
  }
}

}  // namespace
