// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/video_player_custom.h"

#include "gtest/gtest.h"
#include "src/render/texture_info_frame_data.h"
#include "src/render/texture_info_gl.h"
#include "src/resource/asset/video_asset.h"
#include "src/video/custom/video_decoder_custom.h"

using namespace lynx::animax;

namespace {

class MockVideoDecoderCustom : public VideoDecoderCustom {
 public:
  bool IsValid() override { return true; }

  bool CreateDecoder(std::shared_ptr<VideoAsset> asset) override {
    return true;
  }

  bool DestroyDecoder() override { return true; }

  std::shared_ptr<YUVFrameInfo> DecodeFrameData(
      std::vector<uint8_t>& data,
      const std::shared_ptr<YUVFrameInfo>& reusable_frame) override {
    return std::make_shared<YUVFrameInfo>();
  }

  std::unique_ptr<TextureInfo> UpdateTexturesFromYuvFrame(
      const std::shared_ptr<YUVFrameInfo>& frame_info) {
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

// Unlike the all-keyframe mock above, this decoder requires contiguous GOP
// packets and writes the decoded frame identity into a reusable Y plane.
class SequentialGopDecoder : public MockVideoDecoderCustom {
 public:
  std::shared_ptr<YUVFrameInfo> DecodeFrameData(
      std::vector<uint8_t>& data,
      const std::shared_ptr<YUVFrameInfo>& reusable_frame) override {
    if (data.size() != 1) {
      return nullptr;
    }
    const int index = data[0];
    if (index % 12 != 0 && index != decoded_index_ + 1) {
      ++missing_reference_packets_;
      return nullptr;
    }
    decoded_index_ = index;
    ++decoded_packet_count_;
    if (reusable_frame) {
      ++reused_frame_count_;
    }
    auto frame =
        reusable_frame ? reusable_frame : std::make_shared<YUVFrameInfo>();
    frame->SetWidth(1);
    frame->SetHeight(1);
    frame->SetYUVPixFmt(YUVPixFmt::kYUVPixFmtI400);
    frame->SetLineSize(0, 1);
    frame->UpdateChannelData(0, data.data());
    return frame;
  }

  int missing_reference_packets_ = 0;
  int decoded_packet_count_ = 0;
  int reused_frame_count_ = 0;

 private:
  int decoded_index_ = -1;
};

class GopVideoAsset : public MockVideoAsset {
 public:
  GopVideoAsset() : MockVideoAsset(VideoAssetModel()) {}
  int32_t GetFrameCount() const override { return 24; }
  int GetPrevKeyFrame(const int32_t frame) const override {
    return frame < 0 ? 0 : (frame / 12) * 12;
  }
  std::vector<uint8_t> GetVideoParameterSets() const override { return {}; }
  std::vector<uint8_t> GetFrameRawData(int32_t frame) const override {
    return {static_cast<uint8_t>(frame)};
  }
};

void ExpectDecodedFrame(VideoPlayerCustom& player, int frame) {
  auto texture = player.UpdateTexture(frame);
  ASSERT_NE(nullptr, texture) << "frame " << frame;
  auto* frame_data = static_cast<TextureInfoFrameData*>(texture.get());
  const auto& pixels = *frame_data->GetFrameInfo()->GetChannelData(0);
  ASSERT_EQ(1u, pixels.size());
  EXPECT_EQ(frame, pixels[0]);
}

TEST_F(VideoPlayerCustomTest, BackwardSeekReplaysCachedReferencePackets) {
  auto decoder = std::make_unique<SequentialGopDecoder>();
  auto* decoder_ptr = decoder.get();
  VideoPlayerCustom player(std::move(decoder));
  player.AttachAsset(std::make_shared<GopVideoAsset>());

  // The 10-frame cache retains future packets after each backward seek. The
  // old implementation skips them on the next forward request and loses refs.
  // The final requests also exercise last-frame prefetch and loop-around.
  for (int frame : {9, 0, 11, 21, 12, 23, 0, 9, 0, 11}) {
    ExpectDecodedFrame(player, frame);
  }
  EXPECT_EQ(0, decoder_ptr->missing_reference_packets_);
  EXPECT_GT(decoder_ptr->reused_frame_count_, 0);
}

TEST_F(VideoPlayerCustomTest, CachedFramePreservesDecodeFrontierAndPixels) {
  auto decoder = std::make_unique<SequentialGopDecoder>();
  auto* decoder_ptr = decoder.get();
  VideoPlayerCustom player(std::move(decoder));
  player.AttachAsset(std::make_shared<GopVideoAsset>());

  ExpectDecodedFrame(player, 9);
  EXPECT_EQ(11, decoder_ptr->decoded_packet_count_);  // Includes prefetch 10.
  // Frame 1 is the oldest cached entry. Prefetch evicts it after the cache hit;
  // its returned pixels must remain intact until the caller uploads them.
  ExpectDecodedFrame(player, 1);
  EXPECT_EQ(12, decoder_ptr->decoded_packet_count_);  // Prefetch 11 only.
  EXPECT_EQ(nullptr, player.UpdateTexture(1));
  EXPECT_EQ(12, decoder_ptr->decoded_packet_count_);
  ExpectDecodedFrame(player, 9);
  EXPECT_EQ(13, decoder_ptr->decoded_packet_count_);  // Prefetch keyframe 12.
  EXPECT_EQ(0, decoder_ptr->missing_reference_packets_);
  EXPECT_GT(decoder_ptr->reused_frame_count_, 0);
}

}  // namespace
