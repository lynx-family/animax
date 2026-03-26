// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/model/value/gradient_color.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

TEST(GradientColorTest, DefaultConstructor) {
  auto gradient = ValueFactory::Make<GradientColor>();
  EXPECT_TRUE(gradient->IsEmpty());
  EXPECT_EQ(gradient->GetSize(), 0);
  EXPECT_EQ(gradient->GetPositions(), nullptr);
  EXPECT_EQ(gradient->GetColors(), nullptr);
}

TEST(GradientColorTest, ConstructorWithSize) {
  auto gradient = ValueFactory::Make<GradientColor>(3);
  EXPECT_FALSE(gradient->IsEmpty());
  EXPECT_EQ(gradient->GetSize(), 3);
  EXPECT_NE(gradient->GetPositions(), nullptr);
  EXPECT_NE(gradient->GetColors(), nullptr);
}

TEST(GradientColorTest, ConstructorWithArrays) {
  auto positions = std::make_unique<float[]>(3);
  auto colors = std::make_unique<int32_t[]>(3);
  positions[0] = 0.0f;
  positions[1] = 0.5f;
  positions[2] = 1.0f;
  colors[0] = 0xFF0000;
  colors[1] = 0x00FF00;
  colors[2] = 0x0000FF;

  auto gradient = ValueFactory::Make<GradientColor>(std::move(positions),
                                                    std::move(colors), 3);

  EXPECT_FALSE(gradient->IsEmpty());
  EXPECT_EQ(gradient->GetSize(), 3);
  EXPECT_NE(gradient->GetPositions(), nullptr);
  EXPECT_NE(gradient->GetColors(), nullptr);

  EXPECT_FLOAT_EQ(gradient->GetPositions()[0], 0.0f);
  EXPECT_FLOAT_EQ(gradient->GetPositions()[1], 0.5f);
  EXPECT_FLOAT_EQ(gradient->GetPositions()[2], 1.0f);
  EXPECT_EQ(gradient->GetColors()[0], 0xFF0000);
  EXPECT_EQ(gradient->GetColors()[1], 0x00FF00);
  EXPECT_EQ(gradient->GetColors()[2], 0x0000FF);
}

TEST(GradientColorTest, Init) {
  auto gradient = ValueFactory::Make<GradientColor>();
  gradient->Init(2);
  EXPECT_FALSE(gradient->IsEmpty());
  EXPECT_EQ(gradient->GetSize(), 2);
  EXPECT_NE(gradient->GetPositions(), nullptr);
  EXPECT_NE(gradient->GetColors(), nullptr);

  // Test reinitialization with same size
  gradient->Init(2);
  EXPECT_EQ(gradient->GetSize(), 2);

  // Test reinitialization with different size
  gradient->Init(3);
  EXPECT_EQ(gradient->GetSize(), 3);
}

TEST(GradientColorTest, UpdateColors) {
  auto gradient = ValueFactory::Make<GradientColor>(2);
  auto new_colors = std::make_unique<int32_t[]>(2);
  new_colors[0] = 0xFF0000;
  new_colors[1] = 0x00FF00;

  gradient->UpdateColors(std::move(new_colors));
  EXPECT_EQ(gradient->GetColors()[0], 0xFF0000);
  EXPECT_EQ(gradient->GetColors()[1], 0x00FF00);
}

TEST(GradientColorTest, LerpColor) {
  // Create first gradient
  auto pos1 = std::make_unique<float[]>(2);
  auto col1 = std::make_unique<int32_t[]>(2);
  pos1[0] = 0.0f;
  pos1[1] = 1.0f;
  col1[0] = 0xFF0000;  // Red
  col1[1] = 0x0000FF;  // Blue
  auto gradient1 =
      ValueFactory::Make<GradientColor>(std::move(pos1), std::move(col1), 2);

  // Create second gradient
  auto pos2 = std::make_unique<float[]>(2);
  auto col2 = std::make_unique<int32_t[]>(2);
  pos2[0] = 0.0f;
  pos2[1] = 1.0f;
  col2[0] = 0x00FF00;  // Green
  col2[1] = 0xFFFF00;  // Yellow
  auto gradient2 =
      ValueFactory::Make<GradientColor>(std::move(pos2), std::move(col2), 2);

  // Create result gradient
  auto result = ValueFactory::Make<GradientColor>(2);
  result->LerpColor(*gradient1, *gradient2, 0.5f);

  // Verify positions are interpolated
  EXPECT_FLOAT_EQ(result->GetPositions()[0], 0.0f);
  EXPECT_FLOAT_EQ(result->GetPositions()[1], 1.0f);

  // Verify colors are interpolated (exact values may vary based on gamma
  // correction)
  EXPECT_NE(result->GetColors()[0], 0xFF0000);
  EXPECT_NE(result->GetColors()[0], 0x00FF00);
  EXPECT_NE(result->GetColors()[1], 0x0000FF);
  EXPECT_NE(result->GetColors()[1], 0xFFFF00);
}

TEST(GradientColorTest, Copy) {
  auto positions = std::make_unique<float[]>(2);
  auto colors = std::make_unique<int32_t[]>(2);
  positions[0] = 0.0f;
  positions[1] = 1.0f;
  colors[0] = 0xFF0000;
  colors[1] = 0x0000FF;

  auto original = ValueFactory::Make<GradientColor>(std::move(positions),
                                                    std::move(colors), 2);
  auto copy = original->Copy();
  auto* gradient_copy = static_cast<GradientColor*>(copy.get());

  EXPECT_NE(gradient_copy, nullptr);
  EXPECT_FALSE(gradient_copy->IsEmpty());
  EXPECT_EQ(gradient_copy->GetSize(), 2);
  EXPECT_NE(gradient_copy->GetPositions(), nullptr);
  EXPECT_NE(gradient_copy->GetColors(), nullptr);

  EXPECT_FLOAT_EQ(gradient_copy->GetPositions()[0], 0.0f);
  EXPECT_FLOAT_EQ(gradient_copy->GetPositions()[1], 1.0f);
  EXPECT_EQ(gradient_copy->GetColors()[0], 0xFF0000);
  EXPECT_EQ(gradient_copy->GetColors()[1], 0x0000FF);
}

TEST(GradientColorTest, CopyEmpty) {
  auto original = ValueFactory::Make<GradientColor>();
  auto copy = original->Copy();
  auto* gradient_copy = static_cast<GradientColor*>(copy.get());

  EXPECT_NE(gradient_copy, nullptr);
  EXPECT_TRUE(gradient_copy->IsEmpty());
  EXPECT_EQ(gradient_copy->GetSize(), 0);
  EXPECT_EQ(gradient_copy->GetPositions(), nullptr);
  EXPECT_EQ(gradient_copy->GetColors(), nullptr);
}
