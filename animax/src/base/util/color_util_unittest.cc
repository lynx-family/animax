// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/base/util/color_util.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

class ColorUtilTest : public ::testing::Test {
 protected:
  ColorUtilTest() = default;
  ~ColorUtilTest() override = default;
};

// Test parsing hex color strings
TEST_F(ColorUtilTest, ParseHexColor) {
  // Test #RRGGBB format
  EXPECT_EQ(ColorUtil::ParseHexColor("#FF0000"), 0xFFFF0000);
  EXPECT_EQ(ColorUtil::ParseHexColor("#00FF00"), 0xFF00FF00);
  EXPECT_EQ(ColorUtil::ParseHexColor("#0000FF"), 0xFF0000FF);
  EXPECT_EQ(ColorUtil::ParseHexColor("#FFFFFF"), 0xFFFFFFFF);
  EXPECT_EQ(ColorUtil::ParseHexColor("#000000"), 0xFF000000);

  // Test #AARRGGBB format
  EXPECT_EQ(ColorUtil::ParseHexColor("#00FF0000"), 0x00FF0000);
  EXPECT_EQ(ColorUtil::ParseHexColor("#80FF0000"), 0x80FF0000);
  EXPECT_EQ(ColorUtil::ParseHexColor("#FFFF0000"), 0xFFFF0000);

  // Test without # prefix
  EXPECT_EQ(ColorUtil::ParseHexColor("FF0000"), 0xFFFF0000);
  EXPECT_EQ(ColorUtil::ParseHexColor("00FF00"), 0xFF00FF00);
  EXPECT_EQ(ColorUtil::ParseHexColor("0000FF"), 0xFF0000FF);
  EXPECT_EQ(ColorUtil::ParseHexColor("00FF0000"), 0x00FF0000);

  // Test lowercase
  EXPECT_EQ(ColorUtil::ParseHexColor("#ff0000"), 0xFFFF0000);
  EXPECT_EQ(ColorUtil::ParseHexColor("#00ff00"), 0xFF00FF00);
  EXPECT_EQ(ColorUtil::ParseHexColor("#0000ff"), 0xFF0000FF);

  // Test edge cases
  EXPECT_EQ(ColorUtil::ParseHexColor(""), 0);
  EXPECT_EQ(ColorUtil::ParseHexColor("#"), 0);
  EXPECT_EQ(ColorUtil::ParseHexColor("#F"), 0);
  EXPECT_EQ(ColorUtil::ParseHexColor("#FF"), 0);
  EXPECT_EQ(ColorUtil::ParseHexColor("#FFF"), 0);
  EXPECT_EQ(ColorUtil::ParseHexColor("#FFFF"), 0);
  EXPECT_EQ(ColorUtil::ParseHexColor("#FFFFF"), 0);

  // Test invalid characters (should return 0 for invalid hex)
  EXPECT_EQ(ColorUtil::ParseHexColor("#GGGGGG"), 0);
  EXPECT_EQ(ColorUtil::ParseHexColor("#ZZZZZZ"), 0);
  EXPECT_EQ(ColorUtil::ParseHexColor("#12345G"), 0);
  EXPECT_EQ(ColorUtil::ParseHexColor("#1234567"), 0);
  EXPECT_EQ(ColorUtil::ParseHexColor("#123456789"), 0);
}

// Test specific color values
TEST_F(ColorUtilTest, SpecificColorValues) {
  // Red
  EXPECT_EQ(ColorUtil::ParseHexColor("#FF0000"), 0xFFFF0000);
  // Green
  EXPECT_EQ(ColorUtil::ParseHexColor("#00FF00"), 0xFF00FF00);
  // Blue
  EXPECT_EQ(ColorUtil::ParseHexColor("#0000FF"), 0xFF0000FF);
  // Yellow
  EXPECT_EQ(ColorUtil::ParseHexColor("#FFFF00"), 0xFFFFFF00);
  // Cyan
  EXPECT_EQ(ColorUtil::ParseHexColor("#00FFFF"), 0xFF00FFFF);
  // Magenta
  EXPECT_EQ(ColorUtil::ParseHexColor("#FF00FF"), 0xFFFF00FF);
  // White
  EXPECT_EQ(ColorUtil::ParseHexColor("#FFFFFF"), 0xFFFFFFFF);
  // Black
  EXPECT_EQ(ColorUtil::ParseHexColor("#000000"), 0xFF000000);
  // Transparent
  EXPECT_EQ(ColorUtil::ParseHexColor("#00000000"), 0x00000000);
  // Semi-transparent red
  EXPECT_EQ(ColorUtil::ParseHexColor("#80FF0000"), 0x80FF0000);
}
