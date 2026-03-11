// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/yuv_frame_info.h"

#include <vector>

#include "gtest/gtest.h"

using namespace lynx::animax;

class YUVFrameInfoTest : public ::testing::Test {
 protected:
  YUVFrameInfoTest() = default;
  ~YUVFrameInfoTest() override = default;
};

TEST_F(YUVFrameInfoTest, UpdateChannelData) {
  auto frame_info = YUVFrameInfo();
  const int length = 20;
  const int line_size = 32;
  frame_info.SetWidth(length);
  frame_info.SetHeight(length);
  frame_info.SetLineWidth(0, length);
  frame_info.SetLineSize(0, line_size);

  std::vector<uint8_t> raw_data = {};
  for (int i = 0; i < length; i++) {
    raw_data.insert(raw_data.end(), length, 0);
    raw_data.insert(raw_data.end(), line_size - length, 1);
  }

  frame_info.UpdateChannelData(0, raw_data.data());
  for (auto data : *frame_info.GetChannelData(0)) {
    EXPECT_EQ(data, 0);
  }
}
