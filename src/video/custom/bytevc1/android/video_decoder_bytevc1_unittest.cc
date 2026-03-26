// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/bytevc1/android/video_decoder_bytevc1.h"

#include "gtest/gtest.h"
#include "src/render/texture_info_gl.h"
#include "src/resource/asset/video_asset.h"

using namespace lynx::animax;

class VideoDecoderByteVc1Test : public ::testing::Test {
 protected:
  VideoDecoderByteVc1Test() = default;
  ~VideoDecoderByteVc1Test() override = default;
};

TEST_F(VideoDecoderByteVc1Test, Create) {
  auto decoder = std::make_unique<VideoDecoderByteVc1>();
  EXPECT_NE(nullptr, decoder);
  EXPECT_EQ(true, decoder->IsValid());

  std::vector<uint8_t> data = {0, 0, 0, 0};
  std::shared_ptr<YUVFrameInfo> frame_info = std::make_shared<YUVFrameInfo>();
  EXPECT_EQ(frame_info, decoder->DecodeFrameData(data, frame_info));
  EXPECT_NE(nullptr, frame_info);
}
