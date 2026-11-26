// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/path.h"

#include "gtest/gtest.h"
#include "skity/graphic/path.hpp"
#include "src/model/value/base_value.h"
#include "src/render/path_measure.h"

using namespace lynx::animax;

TEST(PathTest, CopyRetainsGeometryWhileOriginalContinuesEditing) {
  Path path;
  path.SetFillType(PathFillType::kEvenOdd);
  path.MoveTo(2, 3);
  path.LineTo(12, 3);
  auto copy = path.Copy();
  auto& copied_path = static_cast<Path&>(*copy);
  path.LineTo(12, 13);
  EXPECT_EQ(copied_path.GetPath().CountVerbs(), 2u);
  EXPECT_EQ(path.GetPath().CountVerbs(), 3u);
  EXPECT_EQ(path.GetPath().GetFillType(), skity::Path::PathFillType::kEvenOdd);
  EXPECT_EQ(copied_path.GetPath().GetFillType(),
            skity::Path::PathFillType::kEvenOdd);
  path.Set(path);
  EXPECT_EQ(path.GetPath().CountVerbs(), 3u);
  path.Reset();
  EXPECT_TRUE(path.IsEmpty());
  path.MoveTo(7, 8);
  EXPECT_EQ(path.GetPath().CountPoints(), 1u);
  EXPECT_EQ(path.GetPath().GetFillType(), skity::Path::PathFillType::kWinding);
  EXPECT_EQ(copied_path.GetPath().CountPoints(), 2u);
}

TEST(PathTest, SelfAppendRetainsBothContoursWithTransform) {
  for (bool transformed : {false, true}) {
    Path path;
    path.MoveTo(0, 0);
    path.LineTo(10, 0);
    Matrix matrix;
    if (transformed) {
      matrix.PreTranslate(20, 5, 0);
      path.AddPath(path, matrix);
    } else {
      path.AddPath(path);
    }
    PathMeasure measure;
    measure.SetPath(path, false);
    EXPECT_FLOAT_EQ(measure.GetLength(), 10);
    ASSERT_TRUE(measure.NextContour());
    EXPECT_FLOAT_EQ(measure.GetLength(), 10);
    PointF point;
    ASSERT_TRUE(measure.GetPosTan(5, &point));
    EXPECT_FLOAT_EQ(point.GetX(), transformed ? 25 : 5);
    EXPECT_FLOAT_EQ(point.GetY(), transformed ? 5 : 0);
    EXPECT_FALSE(measure.NextContour());
  }
}

TEST(PathTest, TrimSegmentsAppendAfterReadingDestination) {
  Path source;
  source.MoveTo(0, 0);
  source.LineTo(100, 0);
  PathMeasure measure;
  measure.SetPath(source, false);
  Path trimmed;
  ASSERT_TRUE(measure.GetSegment(20, 40, trimmed, true));
  EXPECT_EQ(trimmed.GetPath().CountVerbs(), 2u);
  ASSERT_TRUE(measure.GetSegment(60, 80, trimmed, true));
  PathMeasure trimmed_measure;
  trimmed_measure.SetPath(trimmed, false);
  EXPECT_FLOAT_EQ(trimmed_measure.GetLength(), 20);
  ASSERT_TRUE(trimmed_measure.NextContour());
  EXPECT_FLOAT_EQ(trimmed_measure.GetLength(), 20);
  PointF point;
  ASSERT_TRUE(trimmed_measure.GetPosTan(10, &point));
  EXPECT_FLOAT_EQ(point.GetX(), 70);
  EXPECT_FLOAT_EQ(point.GetY(), 0);
  EXPECT_FLOAT_EQ(point.GetZ(), 0);
  EXPECT_FALSE(trimmed_measure.NextContour());
  EXPECT_FLOAT_EQ(measure.GetLength(), 100);
}

TEST(PathTest, TransformAfterReadingRetainsContourAndFillType) {
  Path path;
  path.SetFillType(PathFillType::kEvenOdd);
  path.MoveTo(1, 2);
  path.CubicTo(3, 4, 5, 6, 7, 8);
  const skity::Path original = path.GetPath();
  Matrix transform;
  transform.PreTranslate(10, 20, 0);
  path.Transform(transform);
  path.Offset(-2, -3);
  const auto& transformed = path.GetPath();
  EXPECT_EQ(transformed.CountVerbs(), original.CountVerbs());
  EXPECT_EQ(transformed.GetFillType(), original.GetFillType());
  for (size_t i = 0; i < transformed.CountPoints(); ++i) {
    EXPECT_FLOAT_EQ(transformed.GetPoint(i).x, original.GetPoint(i).x + 8);
    EXPECT_FLOAT_EQ(transformed.GetPoint(i).y, original.GetPoint(i).y + 17);
  }
}

TEST(PathTest, BooleanResultCanReplaceInputAndContinueEditing) {
  Path left;
  left.MoveTo(0, 0);
  left.LineTo(10, 0);
  left.LineTo(10, 10);
  left.LineTo(0, 10);
  left.Close();
  Path right;
  right.Set(left);
  right.Offset(5, 0);
  left.Op(left, right, PathOp::kDifference);
  EXPECT_TRUE(left.GetPath().Contains(2, 5));
  EXPECT_FALSE(left.GetPath().Contains(7, 5));
  EXPECT_TRUE(right.GetPath().Contains(7, 5));
  left.Offset(20, 0);
  EXPECT_TRUE(left.GetPath().Contains(22, 5));
  EXPECT_FALSE(left.GetPath().Contains(2, 5));
}
