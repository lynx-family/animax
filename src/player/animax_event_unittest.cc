// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/player/animax_event.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

class AnimaxEventTest : public ::testing::Test {
 protected:
  AnimaxEventTest() = default;
  ~AnimaxEventTest() override = default;
};

TEST_F(AnimaxEventTest, StringifyEventWarning) {
  EXPECT_STREQ(StringifyEventWarning(EventWarning::kNoWarning), "kNoWarning");
  EXPECT_STREQ(StringifyEventWarning(EventWarning::kInvalidLottieFormat),
               "kInvalidLottieFormat");
  EXPECT_STREQ(StringifyEventWarning(EventWarning::kIllegalAlphaVideoSize),
               "kIllegalAlphaVideoSize");
  EXPECT_STREQ(StringifyEventWarning(EventWarning::kAssetCountOverLimit),
               "kAssetCountOverLimit");
  EXPECT_STREQ(
      StringifyEventWarning(EventWarning::kDynamicResourceCannotBeLoaded),
      "kDynamicResourceCannotBeLoaded");
  EXPECT_STREQ(
      StringifyEventWarning(EventWarning::kStartFrameGreaterThanEndFrame),
      "kStartFrameGreaterThanEndFrame");
  EXPECT_STREQ(StringifyEventWarning(EventWarning::kExecuteBeforeReady),
               "kExecuteBeforeReady");
  EXPECT_STREQ(
      StringifyEventWarning(EventWarning::kIllegalSubscribedStartFrame),
      "kIllegalSubscribedStartFrame");
  EXPECT_STREQ(StringifyEventWarning(EventWarning::kTextLayerFontInvalid),
               "kTextLayerFontInvalid");
  EXPECT_STREQ(StringifyEventWarning(EventWarning::kAssetLoadFailed),
               "kAssetLoadFailed");
}

TEST_F(AnimaxEventTest, EventParamValueBasic) {
  // Test int32_t constructor
  EventParamValue int_val(42);
  EXPECT_EQ(int_val.type, EventParamValue::Type::kInt32);
  ASSERT_TRUE(int_val.int_val.has_value());
  EXPECT_EQ(int_val.int_val.value(), 42);
  // Test double constructor
  EventParamValue double_val(3.14);
  EXPECT_EQ(double_val.type, EventParamValue::Type::kDouble);
  ASSERT_TRUE(double_val.double_val.has_value());
  EXPECT_DOUBLE_EQ(double_val.double_val.value(), 3.14);
  // Test string constructor
  std::string test_str = "hello world";
  EventParamValue string_val(test_str);
  EXPECT_EQ(string_val.type, EventParamValue::Type::kString);
  ASSERT_NE(string_val.string_val, nullptr);
  EXPECT_EQ(*string_val.string_val, test_str);
  // Test string vector constructor
  auto vec = std::make_unique<std::vector<std::string>>();
  vec->push_back("foo");
  vec->push_back("bar");
  EventParamValue vector_val(std::move(vec));
  EXPECT_EQ(vector_val.type, EventParamValue::Type::kStringVector);
  ASSERT_NE(vector_val.string_vector_val, nullptr);
  ASSERT_EQ(vector_val.string_vector_val->size(), 2);
  EXPECT_EQ((*vector_val.string_vector_val)[0], "foo");
  EXPECT_EQ((*vector_val.string_vector_val)[1], "bar");
  // Test default constructor
  EventParamValue default_val;
  EXPECT_EQ(default_val.type, EventParamValue::Type::kNone);
  EXPECT_FALSE(default_val.int_val.has_value());
  EXPECT_FALSE(default_val.double_val.has_value());
  EXPECT_EQ(default_val.string_val, nullptr);
  EXPECT_EQ(default_val.string_vector_val, nullptr);
}
