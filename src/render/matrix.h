// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_MATRIX_H_
#define ANIMAX_SRC_RENDER_MATRIX_H_

#include <memory>

#include "src/model/rect_model.h"

namespace skity {
struct Matrix;
}
namespace lynx {
namespace animax {

class Matrix {
 public:
  Matrix();
  explicit Matrix(skity::Matrix const &m);
  ~Matrix();

  bool IsIdentity() const;

  bool Invert(Matrix &matrix);

  // Inverts the 2D projective transform induced by mapping the z=0 plane
  // through this matrix. Unlike a full 4x4 inverse, the returned matrix can
  // map screen-space points back to their coordinates on the z=0 plane.
  bool InvertZ0Plane(Matrix &matrix) const;

  void MapRect(RectF &rect) const;

  void MapPoints(float *points, int32_t size) const;

  void Reset();

  void Set(Matrix &matrix);

  void SetValues(float *values);

  void PreConcat(Matrix &matrix);

  void PreRotate(float degree);

  void PreRotate(float degree, float px, float py);

  void PreRotateXYZ(float x_degree, float y_degree, float z_degree);

  void PreScale(float x, float y, float z);

  void PreTranslate(float x, float y, float z);

  float GetScale() const;

  skity::Matrix const &GetMatrix() const;

 private:
  std::unique_ptr<skity::Matrix> matrix_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_MATRIX_H_
