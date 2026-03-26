// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/bytevc1/android/video_player_provider_bytevc1.h"

#include "gtest/gtest.h"
#include "src/render/texture_info_gl.h"
#include "src/resource/asset/video_asset.h"
#include "src/video/custom/video_player_custom.h"
#include "src/video/video_player_service.h"

using namespace lynx::animax;

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

class VideoPlayerProviderByteVc1DecTest : public ::testing::Test {
 protected:
  VideoPlayerProviderByteVc1DecTest() = default;
  ~VideoPlayerProviderByteVc1DecTest() override = default;
};

TEST_F(VideoPlayerProviderByteVc1DecTest, Create) {
  auto provider = std::make_shared<VideoPlayerProviderByteVc1>();
  auto ret = VideoPlayerService::GetInstance().RegisterProvider("bytevc1dec",
                                                                provider);
  EXPECT_EQ(true, ret);

  ret = RegisterVideoPlayerProvider("bytevc1dec", provider);
  EXPECT_EQ(false, ret);

  auto ability = AnimaXAbility();
  auto asset = MockVideoAsset::Make(VideoAssetModel());
  auto shader = provider->CreateVideoShader(&ability);
  auto player = provider->CreateVideoPlayer(&ability);
  EXPECT_NE(nullptr, shader);
  EXPECT_NE(nullptr, player);

  auto is_support = provider->IsSupported(&ability, asset);
  EXPECT_EQ(false, is_support);
}
