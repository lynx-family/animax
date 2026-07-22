// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/text_gradient_helper.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "skity/effect/shader.hpp"
#include "src/model/text/text_gradient_model.h"

namespace lynx {
namespace animax {
namespace {

std::unique_ptr<GradientColor> MakeGradientColor() {
  auto positions = std::make_unique<float[]>(2);
  positions[0] = 0.f;
  positions[1] = 1.f;
  auto colors = std::make_unique<int32_t[]>(2);
  colors[0] = static_cast<int32_t>(0xFFFF0000u);
  colors[1] = static_cast<int32_t>(0x000000FFu);
  return std::make_unique<GradientColor>(std::move(positions),
                                         std::move(colors), 2);
}

std::unique_ptr<TextGradientItemModel> MakeGradientItem(
    GradientType type, PointF start, std::unique_ptr<PointF> end,
    std::unique_ptr<PointF> radius_xy, float start_angle = 0.f) {
  return std::make_unique<TextGradientItemModel>(
      type, std::make_unique<PointF>(start), std::move(end),
      std::move(radius_xy), MakeGradientColor(), start_angle);
}

TEST(TextGradientHelperTest, ResolvesLinearCoordinatesFromBoundsSize) {
  auto item = MakeGradientItem(GradientType::kLinear, PointF(0.25f, 0.5f),
                               std::make_unique<PointF>(1.f, 0.f), nullptr);
  EXPECT_NE(nullptr, MakeTextGradientShader(*item, 200.f, 100.f));
}

TEST(TextGradientHelperTest, ResolvesCoordinatesWithBoundsOrigin) {
  auto item = MakeGradientItem(GradientType::kLinear, PointF(0.f, 0.f),
                               std::make_unique<PointF>(1.f, 1.f), nullptr);
  EXPECT_NE(nullptr, MakeTextGradientShader(*item, 100.f, 5.f, 4950.f, 0.f));
}

TEST(TextGradientHelperTest, RadialRadiusXYWinsAndPreservesEllipse) {
  auto item = MakeGradientItem(GradientType::kRadial, PointF(0.5f, 0.25f),
                               std::make_unique<PointF>(1.f, 1.f),
                               std::make_unique<PointF>(0.7027f, 1.3175f));
  auto shader = MakeTextGradientShader(*item, 200.f, 100.f);
  ASSERT_NE(nullptr, shader);
  const auto local_matrix = shader->GetShader()->GetLocalMatrix();
  EXPECT_FLOAT_EQ(140.54f, local_matrix.GetScaleX());
  EXPECT_FLOAT_EQ(131.75f, local_matrix.GetScaleY());
  EXPECT_FLOAT_EQ(100.f, local_matrix.GetTranslateX());
  EXPECT_FLOAT_EQ(25.f, local_matrix.GetTranslateY());
}

TEST(TextGradientHelperTest, EllipticalRadialPreservesBoundsOrigin) {
  auto item = MakeGradientItem(GradientType::kRadial, PointF(0.5f, 0.25f),
                               std::make_unique<PointF>(1.f, 1.f),
                               std::make_unique<PointF>(0.7027f, 1.3175f));
  auto shader = MakeTextGradientShader(*item, 200.f, 100.f, 4950.f, 10.f);
  ASSERT_NE(nullptr, shader);

  const auto local_matrix = shader->GetShader()->GetLocalMatrix();
  EXPECT_FLOAT_EQ(140.54f, local_matrix.GetScaleX());
  EXPECT_FLOAT_EQ(131.75f, local_matrix.GetScaleY());
  EXPECT_FLOAT_EQ(5050.f, local_matrix.GetTranslateX());
  EXPECT_FLOAT_EQ(35.f, local_matrix.GetTranslateY());
}

TEST(TextGradientHelperTest, RadialEndPointDefinesCircularRadius) {
  auto item = MakeGradientItem(GradientType::kRadial, PointF(0.f, 0.f),
                               std::make_unique<PointF>(0.3f, 0.4f), nullptr);
  EXPECT_NE(nullptr, MakeTextGradientShader(*item, 200.f, 100.f));
}

TEST(TextGradientHelperTest, ResolvesCssConicGradient) {
  auto item = MakeGradientItem(GradientType::kConic, PointF(0.5f, 0.5f),
                               nullptr, nullptr, 45.f);
  EXPECT_NE(nullptr, MakeTextGradientShader(*item, 200.f, 100.f));
}

TEST(TextGradientHelperTest, RejectsDegenerateGeometry) {
  auto linear = MakeGradientItem(GradientType::kLinear, PointF(0.5f, 0.5f),
                                 std::make_unique<PointF>(0.5f, 0.5f), nullptr);
  auto radial = MakeGradientItem(GradientType::kRadial, PointF(0.5f, 0.5f),
                                 nullptr, std::make_unique<PointF>(0.f, 1.f));

  EXPECT_EQ(nullptr, MakeTextGradientShader(*linear, 100.f, 100.f));
  EXPECT_EQ(nullptr, MakeTextGradientShader(*radial, 100.f, 100.f));
  EXPECT_EQ(nullptr, MakeTextGradientShader(*linear, 0.f, 100.f));

  auto overflowing = MakeGradientItem(
      GradientType::kLinear, PointF(std::numeric_limits<float>::max(), 0.f),
      std::make_unique<PointF>(1.f, 1.f), nullptr);
  EXPECT_EQ(nullptr, MakeTextGradientShader(*overflowing, 100.f, 100.f));

  auto overflowing_extent = MakeGradientItem(
      GradientType::kLinear, PointF(-std::numeric_limits<float>::max(), 0.f),
      std::make_unique<PointF>(std::numeric_limits<float>::max(), 0.f),
      nullptr);
  EXPECT_EQ(nullptr, MakeTextGradientShader(*overflowing_extent, 1.f, 1.f));
}

}  // namespace
}  // namespace animax
}  // namespace lynx
