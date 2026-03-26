// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/path.h"

#include "skity/graphic/path.hpp"
#include "skity/graphic/path_op.hpp"
#include "src/render/skity_util.h"

namespace lynx {
namespace animax {
namespace {
static skity::PathOp::Op ConvertToSkityPathOp(PathOp op) {
  switch (op) {
    case PathOp::kDifference:
      return skity::PathOp::Op::kDifference;
    case PathOp::kUnion:
      return skity::PathOp::Op::kUnion;
    case PathOp::kIntersect:
      return skity::PathOp::Op::kIntersect;
    case PathOp::kXor:
      return skity::PathOp::Op::kXor;
    default:
      return skity::PathOp::Op::kDifference;
  }
}
}  // namespace

Path::Path() : path_(std::make_unique<skity::Path>()) {}
Path::Path(skity::Path const &path)
    : path_(std::make_unique<skity::Path>(path)) {}
Path::~Path() = default;

skity::Path const &Path::GetPath() const { return *path_; }

skity::Path &Path::GetPath() { return *path_; }

void Path::Set(Path &path) {
  path_->Reset();
  path_->AddPath(*path.path_);
  path_->SetFillType(path.path_->GetFillType());
}

void Path::SetFillType(PathFillType type) {
  if (type == PathFillType::kWinding) {
    path_->SetFillType(skity::Path::PathFillType::kWinding);
  } else {
    path_->SetFillType(skity::Path::PathFillType::kEvenOdd);
  }
}

void Path::MoveTo(float x, float y) { path_->MoveTo(x, y); }

void Path::CubicTo(float x1, float y1, float x2, float y2, float x3, float y3) {
  path_->CubicTo(x1, y1, x2, y2, x3, y3);
}

void Path::LineTo(float x, float y) { path_->LineTo(x, y); }

void Path::Reset() { path_->Reset(); }

void Path::AddPath(Path &path, Matrix &matrix) {
  if (path_->IsEmpty()) {
    // FIXME: FillType is changed when first path is added
    path_->SetFillType(path.path_->GetFillType());
  }

  if (matrix.IsIdentity()) {
    path_->AddPath(*path.path_);
  } else {
    path_->AddPath(*path.path_, matrix.GetMatrix());
  }
}

void Path::AddPath(Path &path) { path_->AddPath(*path.path_); }

void Path::ComputeBounds(RectF &out_bounds, bool exact) const {
  auto rect = path_->GetBounds();

  out_bounds.Set(rect.Left(), rect.Top(), rect.Right(), rect.Bottom());
}

void Path::Transform(Matrix &matrix) {
  *path_ = path_->CopyWithMatrix(matrix.GetMatrix());
}

void Path::Offset(float x, float y) {
  auto matrix = skity::Matrix::Translate(x, y);
  *path_ = path_->CopyWithMatrix(matrix);
}

void Path::ArcTo(const RectF &oval, float start_angle, float sweep_angle,
                 bool force_move_to) {
  auto rect = SkityUtil::MakeSkityRect(oval);

  path_->ArcTo(rect, start_angle, sweep_angle, force_move_to);
}

void Path::Op(Path &path1, Path &path2, PathOp op) {
  path_->Reset();

  if (op == PathOp::kReverseDifference) {
    skity::PathOp::Execute(path2.GetPath(), path1.GetPath(),
                           skity::PathOp::Op::kDifference, path_.get());
  } else {
    skity::PathOp::Execute(path1.GetPath(), path2.GetPath(),
                           ConvertToSkityPathOp(op), path_.get());
  }
}

void Path::Close() { path_->Close(); }

void Path::AddOval(const RectF &oval, PathDirection dir) {
  auto rect = SkityUtil::MakeSkityRect(oval);

  path_->AddOval(rect, dir == PathDirection::kCW
                           ? skity::Path::Direction::kCW
                           : skity::Path::Direction::kCCW);
}

bool Path::IsEmpty() const { return path_->IsEmpty(); }

std::unique_ptr<Value> Path::Copy() const {
  return ValueFactory::Make<Path>(*path_);
}

}  // namespace animax
}  // namespace lynx
