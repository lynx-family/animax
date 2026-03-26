// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_PATH_H_
#define ANIMAX_SRC_RENDER_PATH_H_

#include "src/model/rect_model.h"
#include "src/model/value/base_value.h"
#include "src/render/matrix.h"

namespace skity {
class Path;
}
namespace lynx {
namespace animax {

enum class PathOp : uint8_t {
  kDifference = 0,
  kIntersect,
  kUnion,
  kXor,
  kReverseDifference
};

enum class PathFillType : uint8_t { kWinding = 0, kEvenOdd };

enum class PathDirection : uint8_t {
  kCW = 0,
  kCCW,
};

class Path : public Value {
 public:
  Path();
  explicit Path(skity::Path const& path);
  ~Path();

  ValueType GetValueType() const override { return ValueType::kPath; }

  void Set(Path* path) {
    if (!path) {
      return;
    }
    Set(*path);
  }

  void AddPath(Path* path, Matrix& matrix) {
    if (!path) {
      return;
    }
    AddPath(*path, matrix);
  }

  void AddPath(Path* path) {
    if (!path) {
      return;
    }
    AddPath(*path);
  }

  void Set(Path& path);

  void SetFillType(PathFillType type);

  void MoveTo(float x, float y);

  void CubicTo(float x1, float y1, float x2, float y2, float x3, float y3);

  void LineTo(float x, float y);

  void Reset();

  void AddPath(Path& path, Matrix& matrix);

  void AddPath(Path& path);

  void ComputeBounds(RectF& out_bounds, bool exact) const;

  void Transform(Matrix& matrix);

  void Offset(float x, float y);

  void ArcTo(const RectF& oval, float start_angle, float sweep_angle,
             bool force_move_to);

  void Op(Path& path1, Path& path2, PathOp op);

  void Close();

  void AddOval(const RectF& oval, PathDirection dir);

  bool IsEmpty() const override;

  std::unique_ptr<Value> Copy() const override;

  skity::Path const& GetPath() const;

  skity::Path& GetPath();

 private:
  std::unique_ptr<skity::Path> path_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_PATH_H_
