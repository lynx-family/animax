// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/property/animax_value_param.h"

#include "gtest/gtest.h"
#include "src/model/value/base_value.h"
#include "src/model/value/document_data.h"
#include "src/property/property_type.h"

using namespace lynx::animax;

TEST(AnimaXValueParamTest, ConstructorString) {
  AnimaXValueParam param(std::string("test_string"));

  EXPECT_TRUE(param.IsString());
  EXPECT_FALSE(param.IsNumber());
  EXPECT_FALSE(param.IsBoolean());
  EXPECT_FALSE(param.IsCoordinate());
  EXPECT_FALSE(param.IsColor());
  EXPECT_FALSE(param.IsColorFilter());
  EXPECT_FALSE(param.IsNull());

  EXPECT_EQ(param.GetStringValue(), "test_string");
  EXPECT_EQ(param.GetType(), AnimaXValueParam::Type::kString);
  EXPECT_EQ(param.GetTargetFrame(), AnimaXValueParam::kFrameIndexAll);
}

TEST(AnimaXValueParamTest, ConstructorStringWithTargetFrame) {
  AnimaXValueParam param(std::string("test_string"), 100);

  EXPECT_TRUE(param.IsString());
  EXPECT_EQ(param.GetStringValue(), "test_string");
  EXPECT_EQ(param.GetTargetFrame(), 100);
}

TEST(AnimaXValueParamTest, ConstructorNumber) {
  AnimaXValueParam param(42.5);

  EXPECT_FALSE(param.IsString());
  EXPECT_TRUE(param.IsNumber());
  EXPECT_FALSE(param.IsBoolean());
  EXPECT_FALSE(param.IsCoordinate());
  EXPECT_FALSE(param.IsColor());
  EXPECT_FALSE(param.IsColorFilter());
  EXPECT_FALSE(param.IsNull());

  EXPECT_NEAR(param.GetNumberValue(), 42.5, 0.001);
  EXPECT_EQ(param.GetType(), AnimaXValueParam::Type::kNumber);
  EXPECT_EQ(param.GetApplyMode(), AnimaXValueParam::ApplyMode::kSet);
  EXPECT_EQ(param.GetTargetFrame(), AnimaXValueParam::kFrameIndexAll);
}

TEST(AnimaXValueParamTest, ConstructorNumberWithTargetFrame) {
  AnimaXValueParam param(123.456, AnimaXValueParam::ApplyMode::kAdd, 50);

  EXPECT_TRUE(param.IsNumber());
  EXPECT_NEAR(param.GetNumberValue(), 123.456, 0.001);
  EXPECT_EQ(param.GetApplyMode(), AnimaXValueParam::ApplyMode::kAdd);
  EXPECT_EQ(param.GetTargetFrame(), 50);
}

TEST(AnimaXValueParamTest, ConstructorBooleanTrue) {
  AnimaXValueParam param(true);

  EXPECT_FALSE(param.IsString());
  EXPECT_FALSE(param.IsNumber());
  EXPECT_TRUE(param.IsBoolean());
  EXPECT_FALSE(param.IsCoordinate());
  EXPECT_FALSE(param.IsColor());
  EXPECT_FALSE(param.IsColorFilter());
  EXPECT_FALSE(param.IsNull());

  EXPECT_TRUE(param.GetBooleanValue());
  EXPECT_EQ(param.GetType(), AnimaXValueParam::Type::kBoolean);
}

TEST(AnimaXValueParamTest, ConstructorBooleanFalse) {
  AnimaXValueParam param(false);

  EXPECT_TRUE(param.IsBoolean());
  EXPECT_FALSE(param.GetBooleanValue());
}

TEST(AnimaXValueParamTest, ConstructorBooleanWithTargetFrame) {
  AnimaXValueParam param(true, 75);

  EXPECT_TRUE(param.IsBoolean());
  EXPECT_TRUE(param.GetBooleanValue());
  EXPECT_EQ(param.GetTargetFrame(), 75);
}

TEST(AnimaXValueParamTest, ConstructorCoordinate) {
  AnimaXValueParam param(10.5, 20.7, 0.0);

  EXPECT_FALSE(param.IsString());
  EXPECT_FALSE(param.IsNumber());
  EXPECT_FALSE(param.IsBoolean());
  EXPECT_TRUE(param.IsCoordinate());
  EXPECT_FALSE(param.IsColor());
  EXPECT_FALSE(param.IsColorFilter());
  EXPECT_FALSE(param.IsNull());

  EXPECT_DOUBLE_EQ(param.GetX(), 10.5);
  EXPECT_NEAR(param.GetY(), 20.7, 0.001);
  EXPECT_DOUBLE_EQ(param.GetZ(), 0.0);
  EXPECT_EQ(param.GetType(), AnimaXValueParam::Type::kCoordinate);
  EXPECT_EQ(param.GetApplyMode(), AnimaXValueParam::ApplyMode::kSet);
}

TEST(AnimaXValueParamTest, ConstructorCoordinateWithTargetFrame) {
  AnimaXValueParam param(5.0, 15.0, 0.0, AnimaXValueParam::ApplyMode::kAdd, 30);

  EXPECT_TRUE(param.IsCoordinate());
  EXPECT_DOUBLE_EQ(param.GetX(), 5.0);
  EXPECT_DOUBLE_EQ(param.GetY(), 15.0);
  EXPECT_DOUBLE_EQ(param.GetZ(), 0.0);
  EXPECT_EQ(param.GetApplyMode(), AnimaXValueParam::ApplyMode::kAdd);
  EXPECT_EQ(param.GetTargetFrame(), 30);
}

TEST(AnimaXValueParamTest, ConstructorColor) {
  AnimaXValueParam param(static_cast<int32_t>(0xFF0000FF));

  EXPECT_FALSE(param.IsString());
  EXPECT_FALSE(param.IsNumber());
  EXPECT_FALSE(param.IsBoolean());
  EXPECT_FALSE(param.IsCoordinate());
  EXPECT_TRUE(param.IsColor());
  EXPECT_FALSE(param.IsColorFilter());
  EXPECT_FALSE(param.IsNull());

  EXPECT_EQ(param.GetColorValue(), static_cast<int32_t>(0xFF0000FF));
  EXPECT_EQ(param.GetType(), AnimaXValueParam::Type::kColor);
}

TEST(AnimaXValueParamTest, ConstructorColorWithTargetFrame) {
  AnimaXValueParam param(static_cast<int32_t>(0x00FF00FF), 60);

  EXPECT_TRUE(param.IsColor());
  EXPECT_EQ(param.GetColorValue(), static_cast<int32_t>(0x00FF00FF));
  EXPECT_EQ(param.GetTargetFrame(), 60);
}

TEST(AnimaXValueParamTest, ConstructorColorFilter) {
  AnimaXValueParam::ColorFilterParam filter_params(
      static_cast<int32_t>(0xFF0000FF), 1);
  AnimaXValueParam param(filter_params);

  EXPECT_FALSE(param.IsString());
  EXPECT_FALSE(param.IsNumber());
  EXPECT_FALSE(param.IsBoolean());
  EXPECT_FALSE(param.IsCoordinate());
  EXPECT_FALSE(param.IsColor());
  EXPECT_TRUE(param.IsColorFilter());
  EXPECT_FALSE(param.IsNull());

  EXPECT_EQ(param.GetColorValue(), static_cast<int32_t>(0xFF0000FF));
  EXPECT_EQ(param.GetType(), AnimaXValueParam::Type::kColorFilter);
}

TEST(AnimaXValueParamTest, ConstructorColorFilterWithTargetFrame) {
  AnimaXValueParam::ColorFilterParam filter_params(
      static_cast<int32_t>(0x00FF00FF), 2);
  AnimaXValueParam param(filter_params, 90);

  EXPECT_TRUE(param.IsColorFilter());
  EXPECT_EQ(param.GetColorValue(), static_cast<int32_t>(0x00FF00FF));
  EXPECT_EQ(param.GetTargetFrame(), 90);
}

TEST(AnimaXValueParamTest, ConstructorDefault) {
  AnimaXValueParam param;

  EXPECT_FALSE(param.IsString());
  EXPECT_FALSE(param.IsNumber());
  EXPECT_FALSE(param.IsBoolean());
  EXPECT_FALSE(param.IsCoordinate());
  EXPECT_FALSE(param.IsColor());
  EXPECT_FALSE(param.IsColorFilter());
  EXPECT_TRUE(param.IsNull());

  EXPECT_EQ(param.GetType(), AnimaXValueParam::Type::kNull);
  EXPECT_EQ(param.GetTargetFrame(), AnimaXValueParam::kFrameIndexAll);
}

TEST(AnimaXValueParamTest, MoveConstructor) {
  AnimaXValueParam original(std::string("move_test"));
  AnimaXValueParam moved(std::move(original));

  EXPECT_TRUE(moved.IsString());
  EXPECT_EQ(moved.GetStringValue(), "move_test");
  EXPECT_EQ(moved.GetType(), AnimaXValueParam::Type::kString);
}

TEST(AnimaXValueParamTest, MoveAssignment) {
  AnimaXValueParam original(99.9);
  AnimaXValueParam target;

  target = std::move(original);

  EXPECT_TRUE(target.IsNumber());
  EXPECT_NEAR(target.GetNumberValue(), 99.9, 0.001);
  EXPECT_EQ(target.GetType(), AnimaXValueParam::Type::kNumber);
}

TEST(AnimaXValueParamTest, GetValuePtrString) {
  AnimaXValueParam param(std::string("test"));
  void* ptr = param.GetValuePtr();

  EXPECT_NE(ptr, nullptr);
  auto* string_ptr = static_cast<std::string*>(ptr);
  EXPECT_EQ(*string_ptr, "test");
}

TEST(AnimaXValueParamTest, GetValuePtrNumber) {
  AnimaXValueParam param(42.0);
  void* ptr = param.GetValuePtr();

  EXPECT_NE(ptr, nullptr);
  auto* double_ptr = static_cast<double*>(ptr);
  EXPECT_NEAR(*double_ptr, 42.0, 0.001);
}

TEST(AnimaXValueParamTest, GetValuePtrCoordinate) {
  AnimaXValueParam param(10.0, 20.0, 0.0);
  void* ptr = param.GetValuePtr();

  EXPECT_NE(ptr, nullptr);
  auto* point_ptr = static_cast<PointF*>(ptr);
  EXPECT_DOUBLE_EQ(point_ptr->GetX(), 10.0);
  EXPECT_DOUBLE_EQ(point_ptr->GetY(), 20.0);
}

TEST(AnimaXValueParamTest, GetValuePtrNull) {
  AnimaXValueParam param;
  void* ptr = param.GetValuePtr();

  EXPECT_EQ(ptr, nullptr);
}

TEST(AnimaXValueParamTest, CopyToString) {
  AnimaXValueParam param(std::string("copy_test"));
  std::string target;

  EXPECT_TRUE(param.CopyTo(&target));
  EXPECT_EQ(target, "copy_test");
}

TEST(AnimaXValueParamTest, CopyToStringWrongType) {
  AnimaXValueParam param(42.0);
  std::string target;

  EXPECT_FALSE(param.CopyTo(&target));
}

TEST(AnimaXValueParamTest, CopyToFloat) {
  AnimaXValueParam param(3.14);
  float target;

  EXPECT_TRUE(param.CopyTo(&target));
  EXPECT_FLOAT_EQ(target, 3.14f);
}

TEST(AnimaXValueParamTest, CopyToFloatWrongType) {
  AnimaXValueParam param(std::string("not_number"));
  float target;

  EXPECT_FALSE(param.CopyTo(&target));
}

TEST(AnimaXValueParamTest, CopyToInt32) {
  AnimaXValueParam param(123.0);
  int32_t target;

  EXPECT_TRUE(param.CopyTo(&target));
  EXPECT_EQ(target, 123);
}

TEST(AnimaXValueParamTest, CopyToDouble) {
  AnimaXValueParam param(456.789);
  double target;

  EXPECT_TRUE(param.CopyTo(&target));
  EXPECT_NEAR(target, 456.789, 0.001);
}

TEST(AnimaXValueParamTest, CopyToBoolTrue) {
  AnimaXValueParam param(true);
  bool target;

  EXPECT_TRUE(param.CopyTo(&target));
  EXPECT_TRUE(target);
}

TEST(AnimaXValueParamTest, CopyToBoolFalse) {
  AnimaXValueParam param(false);
  bool target;

  EXPECT_TRUE(param.CopyTo(&target));
  EXPECT_FALSE(target);
}

TEST(AnimaXValueParamTest, CopyToBoolWrongType) {
  AnimaXValueParam param(10.0);
  bool target;

  EXPECT_FALSE(param.CopyTo(&target));
}

TEST(AnimaXValueParamTest, GetStringValueDefault) {
  AnimaXValueParam param(42.0);

  EXPECT_EQ(param.GetStringValue(), "");
}

TEST(AnimaXValueParamTest, GetNumberValueDefault) {
  AnimaXValueParam param(std::string("not_number"));

  EXPECT_NEAR(param.GetNumberValue(), 0.0, 0.001);
}

TEST(AnimaXValueParamTest, GetBooleanValueDefault) {
  AnimaXValueParam param(std::string("not_boolean"));

  EXPECT_FALSE(param.GetBooleanValue());
}

TEST(AnimaXValueParamTest, GetXYZDefault) {
  AnimaXValueParam param(std::string("not_coordinate"));

  EXPECT_DOUBLE_EQ(param.GetX(), 0.0);
  EXPECT_DOUBLE_EQ(param.GetY(), 0.0);
  EXPECT_DOUBLE_EQ(param.GetZ(), 0.0);
}

TEST(AnimaXValueParamTest, GetColorValueDefault) {
  AnimaXValueParam param(std::string("not_color"));

  EXPECT_EQ(param.GetColorValue(), 0u);
}

TEST(AnimaXValueParamTest, ColorFilterParamConstructor) {
  AnimaXValueParam::ColorFilterParam filter_params;

  EXPECT_EQ(filter_params.color, 0);
  EXPECT_EQ(filter_params.mode, 0);
}

TEST(AnimaXValueParamTest, ColorFilterParamConstructorWithValues) {
  AnimaXValueParam::ColorFilterParam filter_params(
      static_cast<int32_t>(0xFF0000FF), 3);

  EXPECT_EQ(filter_params.color, static_cast<int32_t>(0xFF0000FF));
  EXPECT_EQ(filter_params.mode, 3);
}

TEST(AnimaXValueParamTest, FromValueNull) {
  auto param = AnimaXValueParam::FromValue(nullptr);

  EXPECT_NE(param, nullptr);
  EXPECT_TRUE(param->IsNull());
}

TEST(AnimaXValueParamTest, EdgeCaseEmptyString) {
  AnimaXValueParam param(std::string(""));

  EXPECT_TRUE(param.IsString());
  EXPECT_EQ(param.GetStringValue(), "");
}

TEST(AnimaXValueParamTest, EdgeCaseZeroValues) {
  {
    AnimaXValueParam param(0.0);
    EXPECT_TRUE(param.IsNumber());
    EXPECT_NEAR(param.GetNumberValue(), 0.0, 0.001);
  }

  {
    AnimaXValueParam param(0.0, 0.0, 0.0);
    EXPECT_TRUE(param.IsCoordinate());
    EXPECT_DOUBLE_EQ(param.GetX(), 0.0);
    EXPECT_DOUBLE_EQ(param.GetY(), 0.0);
    EXPECT_DOUBLE_EQ(param.GetZ(), 0.0);
  }
}

TEST(AnimaXValueParamTest, EdgeCaseNegativeValues) {
  {
    AnimaXValueParam param(-42.5);
    EXPECT_TRUE(param.IsNumber());
    EXPECT_NEAR(param.GetNumberValue(), -42.5, 0.001);
  }

  {
    AnimaXValueParam param(-10.0, -20.0, 0.0);
    EXPECT_TRUE(param.IsCoordinate());
    EXPECT_DOUBLE_EQ(param.GetX(), -10.0);
    EXPECT_DOUBLE_EQ(param.GetY(), -20.0);
    EXPECT_DOUBLE_EQ(param.GetZ(), 0.0);
  }
}

TEST(AnimaXValueParamTest, EdgeCaseTargetFrameZero) {
  AnimaXValueParam param(std::string("test"), 0);

  EXPECT_EQ(param.GetTargetFrame(), 0);
}

TEST(AnimaXValueParamTest, SelfMoveAssignment) {
  AnimaXValueParam param(std::string("self_move"));
  param = std::move(param);  // Self-assignment

  EXPECT_TRUE(param.IsString());
  EXPECT_EQ(param.GetStringValue(), "self_move");
}

TEST(AnimaXValueParamTest, FromValueFloat) {
  Float value(12.34f);
  auto param = AnimaXValueParam::FromValue(&value);
  EXPECT_TRUE(param->IsNumber());
  EXPECT_NEAR(param->GetNumberValue(), 12.34, 0.001);
}

TEST(AnimaXValueParamTest, FromValueInteger) {
  Integer value(123);
  auto param = AnimaXValueParam::FromValue(&value);
  EXPECT_TRUE(param->IsNumber());
  EXPECT_NEAR(param->GetNumberValue(), 123.0, 0.001);
}

TEST(AnimaXValueParamTest, FromValuePoint) {
  PointF value(1.0f, 2.0f, 3.0f);
  auto param = AnimaXValueParam::FromValue(&value);
  EXPECT_TRUE(param->IsCoordinate());
  EXPECT_DOUBLE_EQ(param->GetX(), 1.0);
  EXPECT_DOUBLE_EQ(param->GetY(), 2.0);
  EXPECT_DOUBLE_EQ(param->GetZ(), 3.0);
}

TEST(AnimaXValueParamTest, FromValueScale) {
  ScaleF value(1.5f, 2.5f);
  auto param = AnimaXValueParam::FromValue(&value);
  EXPECT_TRUE(param->IsCoordinate());
  EXPECT_DOUBLE_EQ(param->GetX(), 1.5);
  EXPECT_DOUBLE_EQ(param->GetY(), 2.5);
}

TEST(AnimaXValueParamTest, FromValueColor) {
  Color value(0xAABBCCDD);
  auto param = AnimaXValueParam::FromValue(&value);
  EXPECT_TRUE(param->IsColor());
  EXPECT_EQ(param->GetColorValue(), static_cast<int32_t>(0xAABBCCDD));
}

TEST(AnimaXValueParamTest, FromValueDocument) {
  auto value = ValueFactory::Make<DocumentData>();
  value->SetText("hello");
  auto param = AnimaXValueParam::FromValue(value.get());
  EXPECT_TRUE(param->IsString());
  EXPECT_EQ(param->GetStringValue(), "hello");
}

TEST(AnimaXValueParamTest, ToValueFloat) {
  AnimaXValueParam param(55.5);
  auto value = param.ToValue(LayerPropertyType::kTransformRotation);
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(value->GetValueType(), ValueType::kFloat);
  EXPECT_FLOAT_EQ(static_cast<Float*>(value.get())->Get(), 55.5f);
}

TEST(AnimaXValueParamTest, ToValueInteger) {
  AnimaXValueParam param(66.0);
  auto value = param.ToValue(LayerPropertyType::kOpacity);
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(value->GetValueType(), ValueType::kInteger);
  EXPECT_EQ(static_cast<Integer*>(value.get())->Get(), 66);
}

TEST(AnimaXValueParamTest, ToValuePoint) {
  AnimaXValueParam param(10.0, 20.0, 30.0);
  auto value = param.ToValue(LayerPropertyType::kTransformPosition);
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(value->GetValueType(), ValueType::kPoint);
  auto* point_value = static_cast<PointF*>(value.get());
  EXPECT_FLOAT_EQ(point_value->GetX(), 10.0f);
  EXPECT_FLOAT_EQ(point_value->GetY(), 20.0f);
  EXPECT_FLOAT_EQ(point_value->GetZ(), 30.0f);
}

TEST(AnimaXValueParamTest, ToValueScale) {
  AnimaXValueParam param(1.5, 2.5, 0.0);
  auto value = param.ToValue(LayerPropertyType::kTransformScale);
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(value->GetValueType(), ValueType::kScale);
  auto* scale_value = static_cast<ScaleF*>(value.get());
  EXPECT_FLOAT_EQ(scale_value->GetX(), 1.5f);
  EXPECT_FLOAT_EQ(scale_value->GetY(), 2.5f);
  EXPECT_FLOAT_EQ(scale_value->GetZ(), 0.0f);
}

TEST(AnimaXValueParamTest, ToValueColor) {
  AnimaXValueParam param(static_cast<int32_t>(0xFF112233));
  auto value = param.ToValue(LayerPropertyType::kColor);
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(value->GetValueType(), ValueType::kColor);
  EXPECT_EQ(static_cast<Color*>(value.get())->GetInt(),
            static_cast<int32_t>(0xFF112233));
}

TEST(AnimaXValueParamTest, ToValueDocument) {
  AnimaXValueParam param(std::string("test_doc"));
  auto value = param.ToValue(LayerPropertyType::kTextValue);
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(value->GetValueType(), ValueType::kDocument);
  EXPECT_EQ(static_cast<DocumentData*>(value.get())->GetText(), "test_doc");
}

TEST(AnimaXValueParamTest, FromValueUnknown) {
  Value value;
  auto param = AnimaXValueParam::FromValue(&value);
  EXPECT_TRUE(param->IsNull());
}

TEST(AnimaXValueParamTest, ToValueUnknownProperty) {
  AnimaXValueParam param(123.0);
  auto value = param.ToValue(LayerPropertyType::kUnknown);
  EXPECT_EQ(value, nullptr);
}

TEST(AnimaXValueParamTest, ToValueWrongType) {
  AnimaXValueParam param(std::string("not_a_number"));
  auto value = param.ToValue(LayerPropertyType::kTransformRotation);
  EXPECT_EQ(value, nullptr);
}
