// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/matrix.h"

#include <array>
#include <cmath>

#include "gtest/gtest.h"
#include "skity/geometry/matrix.hpp"

namespace lynx {
namespace animax {
namespace {

constexpr float kTolerance = 1e-3f;

template <size_t N>
void ExpectRoundTripOnZ0Plane(Matrix& matrix,
                              const std::array<float, N>& points) {
  static_assert(N % 2 == 0);
  auto mapped_points = points;
  matrix.MapPoints(mapped_points.data(), static_cast<int32_t>(N / 2));

  Matrix inverse;
  ASSERT_TRUE(matrix.InvertZ0Plane(inverse));
  inverse.MapPoints(mapped_points.data(), static_cast<int32_t>(N / 2));

  for (size_t i = 0; i < N; ++i) {
    EXPECT_NEAR(mapped_points[i], points[i], kTolerance);
  }
}

TEST(MatrixTest, InvertZ0PlaneMatchesTwoDimensionalTransform) {
  Matrix matrix;
  matrix.PreTranslate(32.f, -18.f, 0.f);
  matrix.PreRotate(27.f);
  matrix.PreScale(1.5f, 0.75f, 1.f);

  ExpectRoundTripOnZ0Plane(
      matrix, std::array<float, 6>{-4.f, 3.f, 0.f, 0.f, 12.f, -7.f});
}

TEST(MatrixTest, InvertZ0PlaneHandlesThreeDimensionalTransform) {
  Matrix matrix;
  matrix.PreTranslate(30.f, -20.f, 40.f);
  matrix.PreRotateXYZ(35.f, 25.f, 15.f);
  matrix.PreScale(1.5f, 0.75f, 2.f);

  ExpectRoundTripOnZ0Plane(
      matrix, std::array<float, 6>{-4.f, 3.f, 0.f, 0.f, 12.f, -7.f});
}

TEST(MatrixTest, InvertZ0PlanePreservesPerspective) {
  skity::Matrix skity_matrix(1.2f, 0.1f, 0.3f, 0.001f,    // First column.
                             -0.2f, 0.9f, 0.4f, -0.002f,  // Second column.
                             0.5f, -0.4f, 1.1f, 0.003f,   // Third column.
                             12.f, -8.f, 20.f,
                             1.f);  // Translation and homogeneous scale.
  Matrix matrix(skity_matrix);

  ExpectRoundTripOnZ0Plane(
      matrix,
      std::array<float, 8>{-20.f, -10.f, 0.f, 0.f, 15.f, 8.f, 40.f, -25.f});
}

TEST(MatrixTest, InvertZ0PlaneRejectsEdgeOnPlane) {
  // This is an invertible 4x4 rotation that maps the local x axis entirely to
  // z. Its z=0 plane therefore collapses to a line in screen space.
  skity::Matrix skity_matrix(0.f, 0.f, 1.f, 0.f,   // First column.
                             0.f, 1.f, 0.f, 0.f,   // Second column.
                             -1.f, 0.f, 0.f, 0.f,  // Third column.
                             0.f, 0.f, 0.f, 1.f);  // Fourth column.
  Matrix matrix(skity_matrix);

  Matrix full_inverse;
  EXPECT_TRUE(matrix.Invert(full_inverse));

  Matrix plane_inverse;
  EXPECT_FALSE(matrix.InvertZ0Plane(plane_inverse));
}

}  // namespace
}  // namespace animax
}  // namespace lynx
