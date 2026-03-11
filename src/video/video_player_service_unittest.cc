// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/video_player_service.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

namespace {

class MockVideoShader : public VideoShader {
 public:
  bool Valid() override { return true; }

  void Init(int32_t w, int32_t h, const std::array<float, 4> &rgb_frame,
            const std::array<float, 4> &a_frame) override {}

  void Draw(std::unique_ptr<TextureInfo> texture_info,
            const std::array<float, 16> &transform) override {}

  std::unique_ptr<Image> GetOutputImage(RealContext *context) override {
    return nullptr;
  }
};

class MockVideoPlayerProvider : public VideoPlayerProvider {
 public:
  std::unique_ptr<VideoPlayer> CreateVideoPlayer(
      const AnimaXAbility *ability) override {
    return nullptr;
  }

  std::unique_ptr<VideoShader> CreateVideoShader() override {
    return std::make_unique<MockVideoShader>();
  }

  bool IsSupported(const AnimaXAbility *ability,
                   const VideoAsset *asset) override {
    return true;
  }
};

class MockVideoAsset : public VideoAsset {
 public:
  using VideoAsset::VideoAsset;  // Inherit constructors
  static std::shared_ptr<VideoAsset> Make(VideoAssetModel model) {
    return std::shared_ptr<VideoAsset>(new MockVideoAsset(std::move(model)));
  }

  bool PrepareFrameData(const std::string &video_path) override {
    return false;
  }

  int32_t GetFrameCount() const override { return 240; }
};

std::unique_ptr<MockVideoPlayerProvider> CreateTestVideoPlayerProvider() {
  return std::make_unique<MockVideoPlayerProvider>();
}

TEST(MockVideoPlayerProvider, CreateVideoPlayer) {
  auto ability = AnimaXAbility();
  auto asset = MockVideoAsset::Make(VideoAssetModel());

  auto provider_0 =
      VideoPlayerService::GetInstance().GetProvider(&ability, asset.get());
  auto player_0 = provider_0->CreateVideoPlayer(&ability);
  auto shader_0 = provider_0->CreateVideoShader();
  EXPECT_EQ(nullptr, player_0);
  EXPECT_EQ(nullptr, shader_0);

  bool ret = VideoPlayerService::GetInstance().RegisterProvider(
      "mock_provider", CreateTestVideoPlayerProvider());
  EXPECT_EQ(true, ret);

  ret = VideoPlayerService::GetInstance().RegisterProvider("mock_provider",
                                                           nullptr);
  EXPECT_EQ(false, ret);

  ret = RegisterVideoPlayerProvider("mock_provider",
                                    CreateTestVideoPlayerProvider());
  EXPECT_EQ(false, ret);

  auto provider_1 =
      VideoPlayerService::GetInstance().GetProvider(&ability, asset.get());
  auto player_1 = provider_1->CreateVideoPlayer(&ability);
  auto shader_1 = provider_1->CreateVideoShader();
  EXPECT_EQ(nullptr, player_1);
  EXPECT_NE(nullptr, shader_1);
}

}  // namespace
