// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/model/value/base_value.h"

#include "gtest/gtest.h"
#include "src/render/color_filter.h"

using namespace lynx::animax;

// Integer Tests
TEST(IntegerTest, ConstructorWithValue) {
  auto value = ValueFactory::Make<Integer>(42);
  EXPECT_EQ(value->Get(), 42);
  EXPECT_FALSE(value->IsEmpty());
}

TEST(IntegerTest, DefaultConstructor) {
  auto value = ValueFactory::Make<Integer>();
  EXPECT_EQ(value->Get(), 0);
  EXPECT_TRUE(value->IsEmpty());
}

TEST(IntegerTest, SetAndReset) {
  auto value = ValueFactory::Make<Integer>();
  value->Set(100);
  EXPECT_EQ(value->Get(), 100);
  EXPECT_FALSE(value->IsEmpty());

  value->Reset();
  EXPECT_EQ(value->Get(), 0);
  EXPECT_TRUE(value->IsEmpty());
}

TEST(IntegerTest, Copy) {
  auto original = ValueFactory::Make<Integer>(42);
  auto copy = original->Copy();
  auto* int_copy = static_cast<Integer*>(copy.get());

  EXPECT_NE(int_copy, nullptr);
  EXPECT_EQ(int_copy->Get(), 42);
  EXPECT_FALSE(int_copy->IsEmpty());
}

TEST(IntegerTest, CopyEmpty) {
  auto original = ValueFactory::Make<Integer>();
  auto copy = original->Copy();
  auto* int_copy = static_cast<Integer*>(copy.get());

  EXPECT_NE(int_copy, nullptr);
  EXPECT_EQ(int_copy->Get(), 0);
  EXPECT_TRUE(int_copy->IsEmpty());
}

// Float Tests
TEST(FloatTest, ConstructorWithValue) {
  auto value = ValueFactory::Make<Float>(3.14f);
  EXPECT_FLOAT_EQ(value->Get(), 3.14f);
  EXPECT_FALSE(value->IsEmpty());
}

TEST(FloatTest, DefaultConstructor) {
  auto value = ValueFactory::Make<Float>();
  EXPECT_FLOAT_EQ(value->Get(), 0.0f);
  EXPECT_TRUE(value->IsEmpty());
}

TEST(FloatTest, SetAndReset) {
  auto value = ValueFactory::Make<Float>();
  value->Set(2.718f);
  EXPECT_FLOAT_EQ(value->Get(), 2.718f);
  EXPECT_FALSE(value->IsEmpty());

  value->Reset();
  EXPECT_FLOAT_EQ(value->Get(), 0.0f);
  EXPECT_TRUE(value->IsEmpty());
}

TEST(FloatTest, Copy) {
  auto original = ValueFactory::Make<Float>(3.14f);
  auto copy = original->Copy();
  auto* float_copy = static_cast<Float*>(copy.get());

  EXPECT_NE(float_copy, nullptr);
  EXPECT_FLOAT_EQ(float_copy->Get(), 3.14f);
  EXPECT_FALSE(float_copy->IsEmpty());
}

TEST(FloatTest, CopyEmpty) {
  auto original = ValueFactory::Make<Float>();
  auto copy = original->Copy();
  auto* float_copy = static_cast<Float*>(copy.get());

  EXPECT_NE(float_copy, nullptr);
  EXPECT_FLOAT_EQ(float_copy->Get(), 0.0f);
  EXPECT_TRUE(float_copy->IsEmpty());
}

// PointF Tests
TEST(PointFTest, ConstructorWithTwoValues) {
  auto point = ValueFactory::Make<PointF>(1.0f, 2.0f);
  EXPECT_FLOAT_EQ(point->GetX(), 1.0f);
  EXPECT_FLOAT_EQ(point->GetY(), 2.0f);
  EXPECT_FLOAT_EQ(point->GetZ(), 0.0f);
  EXPECT_FALSE(point->IsEmpty());
}

TEST(PointFTest, ConstructorWithThreeValues) {
  auto point = ValueFactory::Make<PointF>(1.0f, 2.0f, 3.0f);
  EXPECT_FLOAT_EQ(point->GetX(), 1.0f);
  EXPECT_FLOAT_EQ(point->GetY(), 2.0f);
  EXPECT_FLOAT_EQ(point->GetZ(), 3.0f);
  EXPECT_FALSE(point->IsEmpty());
}

TEST(PointFTest, DefaultConstructor) {
  auto point = ValueFactory::Make<PointF>();
  EXPECT_FLOAT_EQ(point->GetX(), 0.0f);
  EXPECT_FLOAT_EQ(point->GetY(), 0.0f);
  EXPECT_FLOAT_EQ(point->GetZ(), 0.0f);
  EXPECT_TRUE(point->IsEmpty());
}

TEST(PointFTest, SetAndReset) {
  auto point = ValueFactory::Make<PointF>();
  point->Set(3.0f, 4.0f, 5.0f);
  EXPECT_FLOAT_EQ(point->GetX(), 3.0f);
  EXPECT_FLOAT_EQ(point->GetY(), 4.0f);
  EXPECT_FLOAT_EQ(point->GetZ(), 5.0f);
  EXPECT_FALSE(point->IsEmpty());

  point->Reset();
  EXPECT_FLOAT_EQ(point->GetX(), 0.0f);
  EXPECT_FLOAT_EQ(point->GetY(), 0.0f);
  EXPECT_FLOAT_EQ(point->GetZ(), 0.0f);
  EXPECT_TRUE(point->IsEmpty());
}

TEST(PointFTest, Length) {
  auto point2D = ValueFactory::Make<PointF>(3.0f, 4.0f, 0.0f);
  EXPECT_FLOAT_EQ(point2D->Length(), 5.0f);

  auto point3D = ValueFactory::Make<PointF>(3.0f, 4.0f, 12.0f);
  EXPECT_FLOAT_EQ(point3D->Length(), 13.0f);
}

TEST(PointFTest, Equals) {
  auto point1 = ValueFactory::Make<PointF>(1.0f, 2.0f, 3.0f);
  auto point2 = ValueFactory::Make<PointF>(1.0f, 2.0f, 3.0f);
  auto point3 = ValueFactory::Make<PointF>(2.0f, 1.0f, 3.0f);
  auto point4 = ValueFactory::Make<PointF>(1.0f, 2.0f, 4.0f);

  EXPECT_TRUE(point1->Equals(*point2));
  EXPECT_TRUE(point1->Equals(1.0f, 2.0f, 3.0f));
  EXPECT_FALSE(point1->Equals(*point3));
  EXPECT_FALSE(point1->Equals(2.0f, 1.0f, 3.0f));
  EXPECT_FALSE(point1->Equals(*point4));
  EXPECT_FALSE(point1->Equals(1.0f, 2.0f, 4.0f));
}

TEST(PointFTest, Copy) {
  auto original = ValueFactory::Make<PointF>(1.0f, 2.0f, 3.0f);
  auto copy = original->Copy();
  auto* point_copy = static_cast<PointF*>(copy.get());

  EXPECT_NE(point_copy, nullptr);
  EXPECT_FLOAT_EQ(point_copy->GetX(), 1.0f);
  EXPECT_FLOAT_EQ(point_copy->GetY(), 2.0f);
  EXPECT_FLOAT_EQ(point_copy->GetZ(), 3.0f);
  EXPECT_FALSE(point_copy->IsEmpty());
}

// ScaleF Tests
TEST(ScaleFTest, ConstructorWithTwoValues) {
  auto scale = ValueFactory::Make<ScaleF>(2.0f, 3.0f);
  EXPECT_FLOAT_EQ(scale->GetX(), 2.0f);
  EXPECT_FLOAT_EQ(scale->GetY(), 3.0f);
  EXPECT_FLOAT_EQ(scale->GetZ(), 1.0f);
  EXPECT_FALSE(scale->IsEmpty());
}

TEST(ScaleFTest, ConstructorWithThreeValues) {
  auto scale = ValueFactory::Make<ScaleF>(2.0f, 3.0f, 4.0f);
  EXPECT_FLOAT_EQ(scale->GetX(), 2.0f);
  EXPECT_FLOAT_EQ(scale->GetY(), 3.0f);
  EXPECT_FLOAT_EQ(scale->GetZ(), 4.0f);
  EXPECT_FALSE(scale->IsEmpty());
}

TEST(ScaleFTest, DefaultConstructor) {
  auto scale = ValueFactory::Make<ScaleF>();
  EXPECT_FLOAT_EQ(scale->GetX(), 1.0f);
  EXPECT_FLOAT_EQ(scale->GetY(), 1.0f);
  EXPECT_FLOAT_EQ(scale->GetZ(), 1.0f);
  EXPECT_TRUE(scale->IsEmpty());
}

TEST(ScaleFTest, SetAndReset) {
  auto scale = ValueFactory::Make<ScaleF>();
  scale->Set(2.0f, 3.0f, 4.0f);
  EXPECT_FLOAT_EQ(scale->GetX(), 2.0f);
  EXPECT_FLOAT_EQ(scale->GetY(), 3.0f);
  EXPECT_FLOAT_EQ(scale->GetZ(), 4.0f);
  EXPECT_FALSE(scale->IsEmpty());

  scale->Reset();
  EXPECT_FLOAT_EQ(scale->GetX(), 1.0f);
  EXPECT_FLOAT_EQ(scale->GetY(), 1.0f);
  EXPECT_FLOAT_EQ(scale->GetZ(), 1.0f);
  EXPECT_TRUE(scale->IsEmpty());
}

TEST(ScaleFTest, Equals) {
  auto scale1 = ValueFactory::Make<ScaleF>(2.0f, 3.0f, 4.0f);
  auto scale2 = ValueFactory::Make<ScaleF>(2.0f, 3.0f, 4.0f);
  auto scale3 = ValueFactory::Make<ScaleF>(3.0f, 2.0f, 4.0f);
  auto scale4 = ValueFactory::Make<ScaleF>(2.0f, 3.0f, 5.0f);

  EXPECT_TRUE(scale1->Equals(2.0f, 3.0f, 4.0f));
  EXPECT_FALSE(scale1->Equals(3.0f, 2.0f, 4.0f));
  EXPECT_FALSE(scale1->Equals(2.0f, 3.0f, 5.0f));
}

TEST(ScaleFTest, Copy) {
  auto original = ValueFactory::Make<ScaleF>(2.0f, 3.0f, 4.0f);
  auto copy = original->Copy();
  auto* scale_copy = static_cast<ScaleF*>(copy.get());

  EXPECT_NE(scale_copy, nullptr);
  EXPECT_FLOAT_EQ(scale_copy->GetX(), 2.0f);
  EXPECT_FLOAT_EQ(scale_copy->GetY(), 3.0f);
  EXPECT_FLOAT_EQ(scale_copy->GetZ(), 4.0f);
  EXPECT_FALSE(scale_copy->IsEmpty());
}

TEST(OrientationTest, SetAndReset) {
  auto orientation = ValueFactory::Make<Orientation>();
  orientation->Set(2.0f, 3.0f, 4.0f);
  EXPECT_FLOAT_EQ(orientation->GetX(), 2.0f);
  EXPECT_FLOAT_EQ(orientation->GetY(), 3.0f);
  EXPECT_FLOAT_EQ(orientation->GetZ(), 4.0f);
  EXPECT_FALSE(orientation->IsEmpty());

  orientation->Reset();
  EXPECT_FLOAT_EQ(orientation->GetX(), 0.0f);
  EXPECT_FLOAT_EQ(orientation->GetY(), 0.0f);
  EXPECT_FLOAT_EQ(orientation->GetZ(), 0.0f);
  EXPECT_TRUE(orientation->IsEmpty());
}

TEST(OrientationTest, Equals) {
  auto orientation1 = ValueFactory::Make<Orientation>(2.0f, 3.0f, 4.0f);
  auto orientation2 = ValueFactory::Make<Orientation>(3.0f, 2.0f, 4.0f);
  auto orientation3 = ValueFactory::Make<Orientation>(2.0f, 3.0f, 5.0f);
  EXPECT_TRUE(orientation1->Equals(2.0f, 3.0f, 4.0f));
  EXPECT_FALSE(orientation3->Equals(3.0f, 2.0f, 4.0f));
  EXPECT_FALSE(orientation2->Equals(2.0f, 3.0f, 5.0f));
}

TEST(OrientationTest, Copy) {
  auto original = ValueFactory::Make<Orientation>(2.0f, 3.0f, 4.0f);
  auto copy = original->Copy();
  auto* orientation_copy = static_cast<Orientation*>(copy.get());

  EXPECT_NE(orientation_copy, nullptr);
  EXPECT_FLOAT_EQ(orientation_copy->GetX(), 2.0f);
  EXPECT_FLOAT_EQ(orientation_copy->GetY(), 3.0f);
  EXPECT_FLOAT_EQ(orientation_copy->GetZ(), 4.0f);
  EXPECT_FALSE(orientation_copy->IsEmpty());
}

// Color Tests
TEST(ColorTest, DefaultConstructor) {
  auto color = ValueFactory::Make<Color>();
  EXPECT_EQ(color->GetA(), 0);
  EXPECT_EQ(color->GetR(), 0);
  EXPECT_EQ(color->GetG(), 0);
  EXPECT_EQ(color->GetB(), 0);
  EXPECT_TRUE(color->IsEmpty());
}

TEST(ColorTest, ConstructorWithComponents) {
  auto color = ValueFactory::Make<Color>(255, 128, 64, 32);
  EXPECT_EQ(color->GetA(), 255);
  EXPECT_EQ(color->GetR(), 128);
  EXPECT_EQ(color->GetG(), 64);
  EXPECT_EQ(color->GetB(), 32);
  EXPECT_FALSE(color->IsEmpty());
}

TEST(ColorTest, ConstructorWithInt) {
  auto color = ValueFactory::Make<Color>(0xFF408020);
  EXPECT_EQ(color->GetA(), 0xFF);
  EXPECT_EQ(color->GetR(), 0x40);
  EXPECT_EQ(color->GetG(), 0x80);
  EXPECT_EQ(color->GetB(), 0x20);
  EXPECT_FALSE(color->IsEmpty());
}

TEST(ColorTest, ConstructorWithHexString) {
  auto color = ValueFactory::Make<Color>("#408020");
  EXPECT_EQ(color->GetA(), 0xFF);
  EXPECT_EQ(color->GetR(), 0x40);
  EXPECT_EQ(color->GetG(), 0x80);
  EXPECT_EQ(color->GetB(), 0x20);
  EXPECT_FALSE(color->IsEmpty());
}

TEST(ColorTest, ConstructorWithHexStringWithAlpha) {
  auto color = ValueFactory::Make<Color>("#80408020");
  EXPECT_EQ(color->GetA(), 0x80);
  EXPECT_EQ(color->GetR(), 0x40);
  EXPECT_EQ(color->GetG(), 0x80);
  EXPECT_EQ(color->GetB(), 0x20);
  EXPECT_FALSE(color->IsEmpty());
}

TEST(ColorTest, SetAndReset) {
  auto color = ValueFactory::Make<Color>();
  color->Set(255, 128, 64, 32);
  EXPECT_EQ(color->GetA(), 255);
  EXPECT_EQ(color->GetR(), 128);
  EXPECT_EQ(color->GetG(), 64);
  EXPECT_EQ(color->GetB(), 32);
  EXPECT_FALSE(color->IsEmpty());

  color->Reset();
  EXPECT_EQ(color->GetA(), 0);
  EXPECT_EQ(color->GetR(), 0);
  EXPECT_EQ(color->GetG(), 0);
  EXPECT_EQ(color->GetB(), 0);
  EXPECT_TRUE(color->IsEmpty());
}

TEST(ColorTest, SetWithInt) {
  auto color = ValueFactory::Make<Color>();
  color->Set(0xFF408020);
  EXPECT_EQ(color->GetA(), 0xFF);
  EXPECT_EQ(color->GetR(), 0x40);
  EXPECT_EQ(color->GetG(), 0x80);
  EXPECT_EQ(color->GetB(), 0x20);
}

TEST(ColorTest, SetWithString) {
  auto color = ValueFactory::Make<Color>();
  color->Set("#408020");
  EXPECT_EQ(color->GetA(), 0xFF);
  EXPECT_EQ(color->GetR(), 0x40);
  EXPECT_EQ(color->GetG(), 0x80);
  EXPECT_EQ(color->GetB(), 0x20);
}

TEST(ColorTest, ToInt) {
  EXPECT_EQ(Color::ToInt(0xFF, 0x40, 0x80, 0x20), 0xFF408020);
}

TEST(ColorTest, GetInt) {
  auto color = ValueFactory::Make<Color>(0xFF, 0x40, 0x80, 0x20);
  EXPECT_EQ(color->GetInt(), 0xFF408020);
}

TEST(ColorTest, Copy) {
  auto original = ValueFactory::Make<Color>(255, 128, 64, 32);
  auto copy = original->Copy();
  auto* color_copy = static_cast<Color*>(copy.get());

  EXPECT_NE(color_copy, nullptr);
  EXPECT_EQ(color_copy->GetA(), 255);
  EXPECT_EQ(color_copy->GetR(), 128);
  EXPECT_EQ(color_copy->GetG(), 64);
  EXPECT_EQ(color_copy->GetB(), 32);
  EXPECT_FALSE(color_copy->IsEmpty());
}

TEST(ColorTest, CopyEmpty) {
  auto original = ValueFactory::Make<Color>();
  auto copy = original->Copy();
  auto* color_copy = static_cast<Color*>(copy.get());

  EXPECT_NE(color_copy, nullptr);
  EXPECT_EQ(color_copy->GetA(), 0);
  EXPECT_EQ(color_copy->GetR(), 0);
  EXPECT_EQ(color_copy->GetG(), 0);
  EXPECT_EQ(color_copy->GetB(), 0);
  EXPECT_TRUE(color_copy->IsEmpty());
}

// ColorFilter Tests
TEST(ColorFilterTest, DefaultConstructor) {
  auto filter = ValueFactory::Make<ColorFilter>();
  EXPECT_TRUE(filter->IsEmpty());
  EXPECT_EQ(filter->GetColor(), -1);
  EXPECT_EQ(filter->GetMode(), -1);
}

TEST(ColorFilterTest, ConstructorWithValues) {
  auto filter = ValueFactory::Make<ColorFilter>(0xAABBCCDD, 1);
  EXPECT_FALSE(filter->IsEmpty());
  EXPECT_EQ(filter->GetColor(), static_cast<int32_t>(0xAABBCCDD));
  EXPECT_EQ(filter->GetMode(), 1);
}

TEST(ColorFilterTest, SetAndReset) {
  auto filter = ValueFactory::Make<ColorFilter>();
  filter->Set(0xFFEEDDCC, 2);
  EXPECT_FALSE(filter->IsEmpty());
  EXPECT_EQ(filter->GetColor(), static_cast<int32_t>(0xFFEEDDCC));
  EXPECT_EQ(filter->GetMode(), 2);

  // IsEmpty is based on has_value(), so we can't really reset it to empty
  // once set. We can set it to default-like values.
  filter->Set(0, 0);
  EXPECT_FALSE(filter->IsEmpty());
  EXPECT_EQ(filter->GetColor(), 0);
  EXPECT_EQ(filter->GetMode(), 0);
}

TEST(ColorFilterTest, Copy) {
  auto original = ValueFactory::Make<ColorFilter>(0xAABBCCDD, 1);
  auto copy = original->Copy();
  auto* filter_copy = static_cast<ColorFilter*>(copy.get());

  EXPECT_NE(filter_copy, nullptr);
  EXPECT_FALSE(filter_copy->IsEmpty());
  EXPECT_EQ(filter_copy->GetColor(), static_cast<int32_t>(0xAABBCCDD));
  EXPECT_EQ(filter_copy->GetMode(), 1);
}

TEST(ColorFilterTest, CopyEmpty) {
  auto original = ValueFactory::Make<ColorFilter>();
  auto copy = original->Copy();
  auto* filter_copy = static_cast<ColorFilter*>(copy.get());

  EXPECT_NE(filter_copy, nullptr);
  EXPECT_TRUE(filter_copy->IsEmpty());
  EXPECT_EQ(filter_copy->GetColor(), -1);
  EXPECT_EQ(filter_copy->GetMode(), -1);
}
