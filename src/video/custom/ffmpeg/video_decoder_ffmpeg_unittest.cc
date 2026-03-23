// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/ffmpeg/video_decoder_ffmpeg.h"

#include "gtest/gtest.h"
#include "src/video/custom/ffmpeg/video_asset_ffmpeg.h"
#include "src/video/custom/video_decoder_custom.h"
#include "src/video/custom/yuv_frame_info.h"

using namespace lynx::animax;

namespace {

class MockVideoAsset : public VideoAssetFFmpeg {
 public:
  explicit MockVideoAsset(VideoAssetModel model)
      : VideoAssetFFmpeg(std::move(model)){};
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

class VideoDecoderFFmpegTest : public ::testing::Test {
 protected:
  VideoDecoderFFmpegTest() = default;
  ~VideoDecoderFFmpegTest() override = default;
};

TEST_F(VideoDecoderFFmpegTest, Create) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();
  EXPECT_NE(nullptr, decoder.get());
}

TEST_F(VideoDecoderFFmpegTest, InitialState) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();
  EXPECT_FALSE(decoder->IsValid());
}

TEST_F(VideoDecoderFFmpegTest, CreateDecoder) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();
  auto asset = std::make_shared<MockVideoAsset>(VideoAssetModel());

  EXPECT_TRUE(decoder->CreateDecoder(asset));
  EXPECT_TRUE(decoder->IsValid());
}

TEST_F(VideoDecoderFFmpegTest, CreateDecoderWithNull) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();

  EXPECT_FALSE(decoder->CreateDecoder(nullptr));
  EXPECT_FALSE(decoder->IsValid());
}

TEST_F(VideoDecoderFFmpegTest, DestroyDecoder) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();
  auto asset = std::make_shared<MockVideoAsset>(VideoAssetModel());

  EXPECT_TRUE(decoder->CreateDecoder(asset));
  EXPECT_TRUE(decoder->IsValid());

  EXPECT_TRUE(decoder->DestroyDecoder());
  EXPECT_FALSE(decoder->IsValid());
}

TEST_F(VideoDecoderFFmpegTest, DecodeFrameWithoutDecoder) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();
  std::vector<uint8_t> data;
  auto reusable_frame = std::make_shared<YUVFrameInfo>();

  EXPECT_EQ(nullptr, decoder->DecodeFrameData(data, reusable_frame));
}

}  // namespace
