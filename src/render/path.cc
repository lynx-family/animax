// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/path.h"

#include <optional>
#include <utility>

#include "skity/graphic/path.hpp"
#include "skity/graphic/path_builder.hpp"
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

Path::Path()
    : path_(std::make_unique<skity::Path>()),
      builder_(std::make_unique<skity::PathBuilder>()) {}
Path::Path(skity::Path const& path)
    : path_(std::make_unique<skity::Path>(path)),
      builder_(std::make_unique<skity::PathBuilder>()),
      building_(false) {}
Path::~Path() = default;

skity::Path const& Path::GetPath() const {
  if (building_) {
    builder_->DetachTo(path_.get());
    building_ = false;
  }
  return *path_;
}

skity::PathBuilder& Path::GetBuilder() {
  if (!building_) {
    *builder_ = skity::PathBuilder(std::move(*path_));
    building_ = true;
  }
  return *builder_;
}

void Path::Set(Path& path) {
  if (&path == this) {
    return;
  }
  *path_ = path.GetPath();
  builder_->Reset();
  building_ = false;
}

void Path::SetFillType(PathFillType type) {
  if (type == PathFillType::kWinding) {
    GetBuilder().SetFillType(skity::Path::PathFillType::kWinding);
  } else {
    GetBuilder().SetFillType(skity::Path::PathFillType::kEvenOdd);
  }
}

void Path::MoveTo(float x, float y) { GetBuilder().MoveTo(x, y); }

void Path::CubicTo(float x1, float y1, float x2, float y2, float x3, float y3) {
  GetBuilder().CubicTo(x1, y1, x2, y2, x3, y3);
}

void Path::LineTo(float x, float y) { GetBuilder().LineTo(x, y); }

void Path::Reset() { GetBuilder().Reset(); }

void Path::AddPath(Path& path, Matrix& matrix) {
  // Editing moves the cached path, so self-appends need an independent source.
  std::optional<skity::Path> source_copy;
  const auto& source =
      &path == this ? source_copy.emplace(path.GetPath()) : path.GetPath();
  auto& builder = GetBuilder();
  if (builder.IsEmpty()) {
    // FIXME: FillType is changed when first path is added
    builder.SetFillType(source.GetFillType());
  }

  if (matrix.IsIdentity()) {
    builder.AddPath(source);
  } else {
    builder.AddPath(source, matrix.GetMatrix());
  }
}

void Path::AddPath(Path& path) {
  std::optional<skity::Path> source_copy;
  const auto& source =
      &path == this ? source_copy.emplace(path.GetPath()) : path.GetPath();
  GetBuilder().AddPath(source);
}

void Path::ComputeBounds(RectF& out_bounds, bool exact) const {
  auto rect = building_ ? builder_->GetBounds() : path_->GetBounds();

  out_bounds.Set(rect.Left(), rect.Top(), rect.Right(), rect.Bottom());
}

void Path::Transform(Matrix& matrix) {
  GetBuilder().Transform(matrix.GetMatrix());
}

void Path::Offset(float x, float y) {
  auto matrix = skity::Matrix::Translate(x, y);
  GetBuilder().Transform(matrix);
}

void Path::ArcTo(const RectF& oval, float start_angle, float sweep_angle,
                 bool force_move_to) {
  auto rect = SkityUtil::MakeSkityRect(oval);

  GetBuilder().ArcTo(rect, start_angle, sweep_angle, force_move_to);
}

void Path::Op(Path& path1, Path& path2, PathOp op) {
  skity::Path result;

  if (op == PathOp::kReverseDifference) {
    skity::PathOp::Execute(path2.GetPath(), path1.GetPath(),
                           skity::PathOp::Op::kDifference, &result);
  } else {
    skity::PathOp::Execute(path1.GetPath(), path2.GetPath(),
                           ConvertToSkityPathOp(op), &result);
  }
  *builder_ = skity::PathBuilder(std::move(result));
  builder_->DetachTo(path_.get());
  building_ = false;
}

void Path::Close() { GetBuilder().Close(); }

void Path::AddOval(const RectF& oval, PathDirection dir) {
  auto rect = SkityUtil::MakeSkityRect(oval);

  GetBuilder().AddOval(rect, dir == PathDirection::kCW
                                 ? skity::Path::Direction::kCW
                                 : skity::Path::Direction::kCCW);
}

bool Path::IsEmpty() const {
  return building_ ? builder_->IsEmpty() : path_->IsEmpty();
}

std::unique_ptr<Value> Path::Copy() const {
  return ValueFactory::Make<Path>(GetPath());
}

}  // namespace animax
}  // namespace lynx
