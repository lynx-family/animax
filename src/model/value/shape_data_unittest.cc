// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/model/value/shape_data.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

TEST(ShapeDataTest, DefaultConstructor) {
  auto shape = ValueFactory::Make<ShapeData>();
  EXPECT_TRUE(shape->IsEmpty());
  EXPECT_FALSE(shape->IsClosed());
  EXPECT_TRUE(shape->GetCurves().empty());
  EXPECT_TRUE(shape->GetInitialPoint().IsEmpty());
}

TEST(ShapeDataTest, Init) {
  auto shape = ValueFactory::Make<ShapeData>();
  auto initial_point = ValueFactory::Make<PointF>(10.0f, 20.0f);
  shape->Init(*initial_point, true);

  EXPECT_FALSE(shape->IsEmpty());
  EXPECT_TRUE(shape->IsClosed());
  EXPECT_FLOAT_EQ(shape->GetInitialPoint().GetX(), 10.0f);
  EXPECT_FLOAT_EQ(shape->GetInitialPoint().GetY(), 20.0f);
}

TEST(ShapeDataTest, SetInitialPoint) {
  auto shape = ValueFactory::Make<ShapeData>();
  shape->SetInitialPoint(15.0f, 25.0f);

  EXPECT_FALSE(shape->IsEmpty());
  EXPECT_FLOAT_EQ(shape->GetInitialPoint().GetX(), 15.0f);
  EXPECT_FLOAT_EQ(shape->GetInitialPoint().GetY(), 25.0f);
}

TEST(ShapeDataTest, SetClosed) {
  auto shape = ValueFactory::Make<ShapeData>();
  shape->SetClosed(true);
  EXPECT_TRUE(shape->IsClosed());

  shape->SetClosed(false);
  EXPECT_FALSE(shape->IsClosed());
}

TEST(ShapeDataTest, Copy) {
  auto shape = ValueFactory::Make<ShapeData>();
  auto initial_point = ValueFactory::Make<PointF>(10.0f, 20.0f);
  shape->Init(*initial_point, true);

  // Add some curves
  auto& curves = shape->GetCurves();
  auto curve1 = CubicCurveModel::Make();
  curve1.SetControlPoint1(1.0f, 2.0f);
  curve1.SetControlPoint2(3.0f, 4.0f);
  curve1.SetVertex(5.0f, 6.0f);
  curves.push_back(curve1);

  auto copy = shape->Copy();
  auto* shape_copy = static_cast<ShapeData*>(copy.get());

  EXPECT_NE(shape_copy, nullptr);
  EXPECT_FALSE(shape_copy->IsEmpty());
  EXPECT_TRUE(shape_copy->IsClosed());
  EXPECT_FLOAT_EQ(shape_copy->GetInitialPoint().GetX(), 10.0f);
  EXPECT_FLOAT_EQ(shape_copy->GetInitialPoint().GetY(), 20.0f);

  EXPECT_EQ(shape_copy->GetCurves().size(), 1);
  auto& copied_curve = shape_copy->GetCurves()[0];
  EXPECT_FLOAT_EQ(copied_curve.GetControlPoint1().GetX(), 1.0f);
  EXPECT_FLOAT_EQ(copied_curve.GetControlPoint1().GetY(), 2.0f);
  EXPECT_FLOAT_EQ(copied_curve.GetControlPoint2().GetX(), 3.0f);
  EXPECT_FLOAT_EQ(copied_curve.GetControlPoint2().GetY(), 4.0f);
  EXPECT_FLOAT_EQ(copied_curve.GetVertex().GetX(), 5.0f);
  EXPECT_FLOAT_EQ(copied_curve.GetVertex().GetY(), 6.0f);
}

TEST(ShapeDataTest, CopyEmpty) {
  auto original = ValueFactory::Make<ShapeData>();
  auto copy = original->Copy();
  auto* shape_copy = static_cast<ShapeData*>(copy.get());

  EXPECT_NE(shape_copy, nullptr);
  EXPECT_TRUE(shape_copy->IsEmpty());
  EXPECT_FALSE(shape_copy->IsClosed());
  EXPECT_TRUE(shape_copy->GetCurves().empty());
  EXPECT_TRUE(shape_copy->GetInitialPoint().IsEmpty());
}

TEST(ShapeDataTest, InterpolateBetween) {
  // Create first shape
  auto shape1 = ValueFactory::Make<ShapeData>();
  auto initial_point1 = ValueFactory::Make<PointF>(0.0f, 0.0f);
  shape1->Init(*initial_point1, true);

  auto& curves1 = shape1->GetCurves();
  auto curve1 = CubicCurveModel::Make();
  curve1.SetControlPoint1(1.0f, 1.0f);
  curve1.SetControlPoint2(2.0f, 2.0f);
  curve1.SetVertex(3.0f, 3.0f);
  curves1.push_back(curve1);

  // Create second shape
  auto shape2 = ValueFactory::Make<ShapeData>();
  auto initial_point2 = ValueFactory::Make<PointF>(10.0f, 10.0f);
  shape2->Init(*initial_point2, false);

  auto& curves2 = shape2->GetCurves();
  auto curve2 = CubicCurveModel::Make();
  curve2.SetControlPoint1(11.0f, 11.0f);
  curve2.SetControlPoint2(12.0f, 12.0f);
  curve2.SetVertex(13.0f, 13.0f);
  curves2.push_back(curve2);

  // Create result shape
  auto result = ValueFactory::Make<ShapeData>();
  result->InterpolateBetween(*shape1, *shape2, 0.5f);

  // Verify interpolated values
  EXPECT_FLOAT_EQ(result->GetInitialPoint().GetX(), 5.0f);
  EXPECT_FLOAT_EQ(result->GetInitialPoint().GetY(), 5.0f);
  EXPECT_TRUE(result->IsClosed());  // Should be true because shape1 is closed

  EXPECT_EQ(result->GetCurves().size(), 1);
  auto& result_curve = result->GetCurves()[0];
  EXPECT_FLOAT_EQ(result_curve.GetControlPoint1().GetX(), 6.0f);
  EXPECT_FLOAT_EQ(result_curve.GetControlPoint1().GetY(), 6.0f);
  EXPECT_FLOAT_EQ(result_curve.GetControlPoint2().GetX(), 7.0f);
  EXPECT_FLOAT_EQ(result_curve.GetControlPoint2().GetY(), 7.0f);
  EXPECT_FLOAT_EQ(result_curve.GetVertex().GetX(), 8.0f);
  EXPECT_FLOAT_EQ(result_curve.GetVertex().GetY(), 8.0f);
}

TEST(ShapeDataTest, InterpolateBetweenDifferentSizes) {
  // Create first shape with one curve
  auto shape1 = ValueFactory::Make<ShapeData>();
  auto initial_point1 = ValueFactory::Make<PointF>(0.0f, 0.0f);
  shape1->Init(*initial_point1, true);

  auto& curves1 = shape1->GetCurves();
  auto curve1 = CubicCurveModel::Make();
  curve1.SetControlPoint1(1.0f, 1.0f);
  curve1.SetControlPoint2(2.0f, 2.0f);
  curve1.SetVertex(3.0f, 3.0f);
  curves1.push_back(curve1);

  // Create second shape with two curves
  auto shape2 = ValueFactory::Make<ShapeData>();
  auto initial_point2 = ValueFactory::Make<PointF>(10.0f, 10.0f);
  shape2->Init(*initial_point2, false);

  auto& curves2 = shape2->GetCurves();
  auto curve2 = CubicCurveModel::Make();
  curve2.SetControlPoint1(11.0f, 11.0f);
  curve2.SetControlPoint2(12.0f, 12.0f);
  curve2.SetVertex(13.0f, 13.0f);
  curves2.push_back(curve2);

  auto curve3 = CubicCurveModel::Make();
  curve3.SetControlPoint1(21.0f, 21.0f);
  curve3.SetControlPoint2(22.0f, 22.0f);
  curve3.SetVertex(23.0f, 23.0f);
  curves2.push_back(curve3);

  // Create result shape
  auto result = ValueFactory::Make<ShapeData>();
  result->InterpolateBetween(*shape1, *shape2, 0.5f);

  // Should only interpolate the first curve since shapes have different sizes
  EXPECT_EQ(result->GetCurves().size(), 1);
  auto& result_curve = result->GetCurves()[0];
  EXPECT_FLOAT_EQ(result_curve.GetControlPoint1().GetX(), 6.0f);
  EXPECT_FLOAT_EQ(result_curve.GetControlPoint1().GetY(), 6.0f);
  EXPECT_FLOAT_EQ(result_curve.GetControlPoint2().GetX(), 7.0f);
  EXPECT_FLOAT_EQ(result_curve.GetControlPoint2().GetY(), 7.0f);
  EXPECT_FLOAT_EQ(result_curve.GetVertex().GetX(), 8.0f);
  EXPECT_FLOAT_EQ(result_curve.GetVertex().GetY(), 8.0f);
}
