// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/matrix.h"

#include <cmath>
#include <cstring>
#include <vector>

#include "skity/geometry/matrix.hpp"
#include "skity/geometry/quaternion.hpp"
#include "src/content/path/path_util.h"
#include "src/render/skity_util.h"

namespace lynx {
namespace animax {

Matrix::Matrix() : matrix_(std::make_unique<skity::Matrix>()) {}
Matrix::Matrix(skity::Matrix const &m)
    : matrix_(std::make_unique<skity::Matrix>(m)) {}
Matrix::~Matrix() = default;

skity::Matrix const &Matrix::GetMatrix() const { return *matrix_; }

bool Matrix::IsIdentity() const { return matrix_->IsIdentity(); }

bool Matrix::Invert(Matrix &matrix) {
  return matrix_->Invert(matrix.matrix_.get());
}

bool Matrix::InvertZ0Plane(Matrix &matrix) const {
  float values[9];
  matrix_->Get9(values);

  skity::Matrix z0_plane_matrix;
  z0_plane_matrix.Set9(values);
  return z0_plane_matrix.Invert(matrix.matrix_.get());
}

void Matrix::MapRect(RectF &rect) const {
  skity::Rect dst{};
  skity::Rect src = SkityUtil::MakeSkityRect(rect);

  matrix_->MapRect(&dst, src);

  rect.Set(dst.Left(), dst.Top(), dst.Right(), dst.Bottom());
}

void Matrix::MapPoints(float *points, int32_t size) const {
  std::vector<skity::Vec2> skity_points;
  std::vector<skity::Vec2> skity_out{static_cast<size_t>(size)};

  for (int32_t i = 0; i < size; i++) {
    skity_points.emplace_back(skity::Vec2(points[i * 2], points[i * 2 + 1]));
  }

  matrix_->MapPoints(skity_out.data(), skity_points.data(),
                     static_cast<int>(skity_points.size()));

  std::memcpy(points, skity_out.data(), size * sizeof(skity::Vec2));
}

void Matrix::Reset() { matrix_->Reset(); }

void Matrix::Set(Matrix &matrix) { *matrix_ = *matrix.matrix_; }

void Matrix::SetValues(float *values) { matrix_->Set9(values); }

void Matrix::PreConcat(Matrix &matrix) { matrix_->PreConcat(*matrix.matrix_); }

void Matrix::PreRotate(float degree) { matrix_->PreRotate(degree); }

void Matrix::PreRotate(float degree, float px, float py) {
  matrix_->PreRotate(degree, px, py);
}

void Matrix::PreRotateXYZ(float x_degree, float y_degree, float z_degree) {
  const auto x_radian = PathUtil::ToRadians(x_degree);
  const auto y_radian = PathUtil::ToRadians(y_degree);
  const auto z_radian = PathUtil::ToRadians(z_degree);
  const auto matrix =
      skity::Quaternion::EulerToMatrix(x_radian, y_radian, z_radian);
  matrix_->PreConcat(matrix);
}

void Matrix::PreScale(float x, float y, float z) {
  skity::Matrix m(1.f);
  m[0][0] = x;
  m[1][1] = y;
  m[2][2] = z;
  matrix_->PreConcat(m);
}

void Matrix::PreTranslate(float x, float y, float z) {
  skity::Matrix m(1.f);
  m[3][0] = x;
  m[3][1] = y;
  m[3][2] = z;
  matrix_->PreConcat(m);
}

float Matrix::GetScale() const {
  float inv_sqrt_2 = std::sqrt(2) / 2.0;
  float points[4];
  points[0] = 0;
  points[1] = 0;
  points[2] = inv_sqrt_2;
  points[3] = inv_sqrt_2;
  MapPoints(points, 2);

  float dx = points[2] - points[0];
  float dy = points[3] - points[1];

  return std::hypot(dx, dy);
}
}  // namespace animax
}  // namespace lynx
