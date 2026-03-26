// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/misc_util.h"

#include "gtest/gtest.h"

namespace lynx {
namespace animax {

class MiscUtilTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(MiscUtilTest, LerpFloat) {
  EXPECT_FLOAT_EQ(5.0f, Lerp(0.0f, 10.0f, 0.5f));
}

TEST_F(MiscUtilTest, LerpInt) { EXPECT_EQ(5, Lerp(0, 10, 0.5f)); }

TEST_F(MiscUtilTest, FloorMod) { EXPECT_EQ(1, FloorMod(10, 3)); }

TEST_F(MiscUtilTest, IsUrlValid) {
  EXPECT_TRUE(IsUrlValid("http://example.com"));
  EXPECT_TRUE(IsUrlValid("asset://image.png"));
  EXPECT_FALSE(IsUrlValid("ftp://example.com"));
}

TEST_F(MiscUtilTest, GammaEvaluateColor) {
  Color start(0, 0, 0, 0);
  Color end(255, 255, 255, 255);
  Color result;
  float progress = 0.5f;
  GammaEvaluate(start, end, progress, &result);

  // Assuming halfway between start and end colors
  EXPECT_EQ(127, result.GetA());
  EXPECT_EQ(127, result.GetR());
  EXPECT_EQ(127, result.GetG());
  EXPECT_EQ(127, result.GetB());
}

// GammaEvaluate() function test for int32_t overloads
TEST_F(MiscUtilTest, GammaEvaluateInt) {
  int32_t start = 0x00000000;  // Black in RGBA
  int32_t end = 0xFFFFFFFF;    // White in RGBA
  float progress = 0.5f;
  int32_t result = GammaEvaluate(start, end, progress);

  // Assuming halfway between start and end colors
  int32_t expected = Color::ToInt(127, 127, 127, 127);
  EXPECT_EQ(expected, result);
}

}  // namespace animax
}  // namespace lynx
