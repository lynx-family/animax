// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/property_update_context.h"

#include "gtest/gtest.h"
#include "include/property/animax_key_path.h"
#include "include/property/animax_value_param.h"
#include "src/property/animax_frame_info.h"

using namespace lynx::animax;

class MockAnimaXValueCallback : public AnimaXValueCallback {
 public:
  std::unique_ptr<Value> GetValue(const Value* original_value,
                                  const AnimaXFrameInfo& frame_info) override {
    return nullptr;
  }
};

class PropertyUpdateContextTest : public ::testing::Test {
 protected:
  void SetUp() override {
    callback_invoked_ = false;
    response_received_ = PropertyUpdateResponse();
  }

  AnimaXPropertyCallback CreateTestCallback() {
    return [this](const PropertyUpdateResponse& response) {
      callback_invoked_ = true;
      response_received_ = response;
    };
  }

  bool callback_invoked_;
  PropertyUpdateResponse response_received_;
};

TEST_F(PropertyUpdateContextTest, LayerStaticRequestConstructor) {
  auto key_path = std::make_unique<AnimaXKeyPath>(
      std::vector<std::string>{"layer1", "transform"});
  auto value = std::make_unique<AnimaXValueParam>(42.5);
  auto callback = CreateTestCallback();

  auto request = LayerStaticRequest::Make(LayerPropertyType::kTransformRotation,
                                          std::move(key_path), std::move(value),
                                          std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_TRUE(context.IsLayerRequest());
  EXPECT_TRUE(context.IsStaticLayerRequest());
  EXPECT_FALSE(context.IsResourceRequest());
  EXPECT_FALSE(context.IsLayerCallbackRequest());
  EXPECT_FALSE(context.HasValueCallback());
  EXPECT_FALSE(context.IsValueNull());
  EXPECT_EQ(context.GetLayerType(), LayerPropertyType::kTransformRotation);
}

TEST_F(PropertyUpdateContextTest, ResourceRequestConstructor) {
  auto value =
      std::make_unique<AnimaXValueParam>(std::string("test_image.png"));
  auto callback = CreateTestCallback();

  auto request = ResourceUpdateRequest::Make(
      ResourcePropertyType::kImageFileName, "resource_123", std::move(value),
      std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_FALSE(context.IsLayerRequest());
  EXPECT_FALSE(context.IsStaticLayerRequest());
  EXPECT_TRUE(context.IsResourceRequest());
  EXPECT_FALSE(context.IsLayerCallbackRequest());
  EXPECT_FALSE(context.HasValueCallback());
  EXPECT_FALSE(context.IsValueNull());
  EXPECT_EQ(context.GetResourceType(), ResourcePropertyType::kImageFileName);
  EXPECT_EQ(context.GetResourceId(), "resource_123");
}

TEST_F(PropertyUpdateContextTest, LayerCallbackRequestConstructor) {
  auto key_path = std::make_unique<AnimaXKeyPath>(
      std::vector<std::string>{"layer1", "color"});
  auto value_callback = std::make_shared<MockAnimaXValueCallback>();
  auto callback = CreateTestCallback();

  auto request = LayerCallbackRequest::Make(
      LayerPropertyType::kColor, std::move(key_path), std::move(value_callback),
      std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_TRUE(context.IsLayerRequest());
  EXPECT_FALSE(context.IsStaticLayerRequest());
  EXPECT_FALSE(context.IsResourceRequest());
  EXPECT_TRUE(context.IsLayerCallbackRequest());
  EXPECT_TRUE(context.HasValueCallback());
  EXPECT_FALSE(context.IsValueNull());
  EXPECT_EQ(context.GetLayerType(), LayerPropertyType::kColor);
  EXPECT_NE(context.GetValueCallback(), nullptr);
}

TEST_F(PropertyUpdateContextTest, InvokeErrorCallback) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value = std::make_unique<AnimaXValueParam>(100.0);
  auto callback = CreateTestCallback();

  auto request =
      LayerStaticRequest::Make(LayerPropertyType::kOpacity, std::move(key_path),
                               std::move(value), std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_FALSE(callback_invoked_);
  context.InvokeErrorCallback(PropertyUpdateResult::kPropertyNotFound);
  EXPECT_TRUE(callback_invoked_);
  EXPECT_FALSE(response_received_.IsSuccess());
  EXPECT_EQ(response_received_.GetErrorType(),
            PropertyUpdateResult::kPropertyNotFound);
}

TEST_F(PropertyUpdateContextTest, InvokeCallbackSuccess) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value = std::make_unique<AnimaXValueParam>(50.0);
  auto callback = CreateTestCallback();

  auto request =
      LayerStaticRequest::Make(LayerPropertyType::kOpacity, std::move(key_path),
                               std::move(value), std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_FALSE(callback_invoked_);
  context.InvokeCallback();
  EXPECT_TRUE(callback_invoked_);
  EXPECT_TRUE(response_received_.IsSuccess());
}

TEST_F(PropertyUpdateContextTest, SetErrorTypeAndGetResponse) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value = std::make_unique<AnimaXValueParam>(75.0);
  auto callback = CreateTestCallback();

  auto request =
      LayerStaticRequest::Make(LayerPropertyType::kOpacity, std::move(key_path),
                               std::move(value), std::move(callback));

  PropertyUpdateContext context(std::move(request));

  context.SetErrorType(PropertyUpdateResult::kValueInvalid);

  PropertyUpdateResponse& response = context.GetResponse();
  EXPECT_FALSE(response.IsSuccess());
  EXPECT_EQ(response.GetErrorType(), PropertyUpdateResult::kValueInvalid);
}

TEST_F(PropertyUpdateContextTest, LayerStaticRequestGetters) {
  std::vector<std::string> path_keys = {"layer1", "transform", "rotation"};
  auto key_path = std::make_unique<AnimaXKeyPath>(path_keys);
  auto value = std::make_unique<AnimaXValueParam>(90.0);
  auto callback = CreateTestCallback();

  auto request = LayerStaticRequest::Make(LayerPropertyType::kTransformRotation,
                                          std::move(key_path), std::move(value),
                                          std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_EQ(context.GetLayerType(), LayerPropertyType::kTransformRotation);
  EXPECT_EQ(context.GetKeyPath().GetKeys(), path_keys);
  EXPECT_TRUE(context.GetValue().IsNumber());
  EXPECT_NEAR(context.GetValue().GetNumberValue(), 90.0, 0.001);
}

TEST_F(PropertyUpdateContextTest, ResourceRequestGetters) {
  auto value = std::make_unique<AnimaXValueParam>(1920.0);
  auto callback = CreateTestCallback();

  auto request = ResourceUpdateRequest::Make(
      ResourcePropertyType::kImageWidth, "image_resource_456", std::move(value),
      std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_EQ(context.GetResourceType(), ResourcePropertyType::kImageWidth);
  EXPECT_EQ(context.GetResourceId(), "image_resource_456");
  EXPECT_TRUE(context.GetValue().IsNumber());
  EXPECT_NEAR(context.GetValue().GetNumberValue(), 1920.0, 0.001);
}

TEST_F(PropertyUpdateContextTest, LayerCallbackRequestGetters) {
  std::vector<std::string> path_keys = {"layer1", "stroke", "color"};
  auto key_path = std::make_unique<AnimaXKeyPath>(path_keys);
  auto value_callback = std::make_shared<MockAnimaXValueCallback>();
  auto callback = CreateTestCallback();

  auto request = LayerCallbackRequest::Make(LayerPropertyType::kStrokeColor,
                                            std::move(key_path), value_callback,
                                            std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_EQ(context.GetLayerType(), LayerPropertyType::kStrokeColor);
  EXPECT_EQ(context.GetKeyPath().GetKeys(), path_keys);
  EXPECT_EQ(context.GetValueCallback(), value_callback);
}

TEST_F(PropertyUpdateContextTest, IsValueNullLayerStaticWithNullValue) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value = std::make_unique<AnimaXValueParam>();
  auto callback = CreateTestCallback();

  auto request =
      LayerStaticRequest::Make(LayerPropertyType::kOpacity, std::move(key_path),
                               std::move(value), std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_TRUE(context.IsValueNull());
}

TEST_F(PropertyUpdateContextTest, IsValueNullResourceWithNullValue) {
  auto value = std::make_unique<AnimaXValueParam>();
  auto callback = CreateTestCallback();

  auto request = ResourceUpdateRequest::Make(ResourcePropertyType::kImageWidth,
                                             "resource_123", std::move(value),
                                             std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_TRUE(context.IsValueNull());
}

TEST_F(PropertyUpdateContextTest, IsValueNullLayerCallbackWithNullCallback) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto callback = CreateTestCallback();

  auto request =
      LayerCallbackRequest::Make(LayerPropertyType::kColor, std::move(key_path),
                                 nullptr, std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_TRUE(context.IsValueNull());
  EXPECT_FALSE(context.HasValueCallback());
}

TEST_F(PropertyUpdateContextTest, LayerRequestTypes) {
  {
    auto key_path =
        std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
    auto value = std::make_unique<AnimaXValueParam>(42.0);
    auto callback = CreateTestCallback();

    auto request = LayerStaticRequest::Make(
        LayerPropertyType::kOpacity, std::move(key_path), std::move(value),
        std::move(callback));

    PropertyUpdateContext context(std::move(request));
    EXPECT_TRUE(context.IsLayerRequest());
  }

  {
    auto key_path =
        std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
    auto value_callback = std::make_shared<MockAnimaXValueCallback>();
    auto callback = CreateTestCallback();

    auto request = LayerCallbackRequest::Make(
        LayerPropertyType::kColor, std::move(key_path),
        std::move(value_callback), std::move(callback));

    PropertyUpdateContext context(std::move(request));
    EXPECT_TRUE(context.IsLayerRequest());
  }

  {
    auto value = std::make_unique<AnimaXValueParam>(std::string("test.png"));
    auto callback = CreateTestCallback();

    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kImageFileName, "resource_123", std::move(value),
        std::move(callback));

    PropertyUpdateContext context(std::move(request));
    EXPECT_FALSE(context.IsLayerRequest());
  }
}

TEST_F(PropertyUpdateContextTest, MultipleErrorTypes) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value = std::make_unique<AnimaXValueParam>(100.0);
  auto callback = CreateTestCallback();

  auto request =
      LayerStaticRequest::Make(LayerPropertyType::kOpacity, std::move(key_path),
                               std::move(value), std::move(callback));

  PropertyUpdateContext context(std::move(request));

  struct TestCase {
    PropertyUpdateResult error_type;
    std::string description;
  };

  std::vector<TestCase> test_cases = {
      {PropertyUpdateResult::kPropertyNull, "property_null"},
      {PropertyUpdateResult::kPropertyInvalidIndex, "property_invalid_index"},
      {PropertyUpdateResult::kValueInvalid, "value_invalid"},
      {PropertyUpdateResult::kKeyPathMismatched, "keypath_mismatched"},
      {PropertyUpdateResult::kResourceNotFound, "resource_not_found"}};

  for (const auto& test_case : test_cases) {
    context.SetErrorType(test_case.error_type);
    PropertyUpdateResponse& response = context.GetResponse();
    EXPECT_FALSE(response.IsSuccess());
    EXPECT_EQ(response.GetErrorType(), test_case.error_type)
        << "Failed for error type: " << test_case.description;
  }
}

TEST_F(PropertyUpdateContextTest, ComplexKeyPathLayerStatic) {
  std::vector<std::string> complex_path = {"root_layer", "**",   "group", "*",
                                           "shape",      "fill", "color"};
  auto key_path = std::make_unique<AnimaXKeyPath>(complex_path);
  auto value =
      std::make_unique<AnimaXValueParam>(static_cast<int32_t>(0xFFFF0000));
  auto callback = CreateTestCallback();

  auto request =
      LayerStaticRequest::Make(LayerPropertyType::kColor, std::move(key_path),
                               std::move(value), std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_EQ(context.GetKeyPath().GetKeys(), complex_path);
  EXPECT_TRUE(context.GetValue().IsColor());
  EXPECT_EQ(context.GetValue().GetColorValue(),
            static_cast<int32_t>(0xFFFF0000));
}

TEST_F(PropertyUpdateContextTest, ComplexKeyPathLayerCallback) {
  std::vector<std::string> complex_path = {"layer", "**", "group", "transform"};
  auto key_path = std::make_unique<AnimaXKeyPath>(complex_path);
  auto value_callback = std::make_shared<MockAnimaXValueCallback>();
  auto callback = CreateTestCallback();

  auto request = LayerCallbackRequest::Make(
      LayerPropertyType::kTransformPosition, std::move(key_path),
      std::move(value_callback), std::move(callback));

  PropertyUpdateContext context(std::move(request));

  EXPECT_EQ(context.GetKeyPath().GetKeys(), complex_path);
  EXPECT_TRUE(context.HasValueCallback());
  EXPECT_NE(context.GetValueCallback(), nullptr);
}

TEST_F(PropertyUpdateContextTest, PointValueLayerStatic) {
  auto key_path = std::make_unique<AnimaXKeyPath>(
      std::vector<std::string>{"layer", "transform"});
  auto value = std::make_unique<AnimaXValueParam>(100.5, 200.7);
  auto callback = CreateTestCallback();

  auto request = LayerStaticRequest::Make(LayerPropertyType::kTransformPosition,
                                          std::move(key_path), std::move(value),
                                          std::move(callback));

  PropertyUpdateContext context(std::move(request));

  const auto& retrieved_value = context.GetValue();
  EXPECT_TRUE(retrieved_value.IsCoordinate());
  EXPECT_DOUBLE_EQ(retrieved_value.GetX(), 100.5);
  EXPECT_NEAR(retrieved_value.GetY(), 200.7, 0.001);
}

TEST_F(PropertyUpdateContextTest, StringValueResource) {
  auto value = std::make_unique<AnimaXValueParam>(std::string("new_font.ttf"));
  auto callback = CreateTestCallback();

  auto request = ResourceUpdateRequest::Make(
      ResourcePropertyType::kFontPath, "font_resource_789", std::move(value),
      std::move(callback));

  PropertyUpdateContext context(std::move(request));

  const auto& retrieved_value = context.GetValue();
  EXPECT_TRUE(retrieved_value.IsString());
  EXPECT_EQ(retrieved_value.GetStringValue(), "new_font.ttf");
}

TEST_F(PropertyUpdateContextTest, CallbackInvocationResourceRequest) {
  auto value = std::make_unique<AnimaXValueParam>(1080.0);
  auto callback = CreateTestCallback();

  auto request = ResourceUpdateRequest::Make(ResourcePropertyType::kVideoHeight,
                                             "video_resource", std::move(value),
                                             std::move(callback));

  PropertyUpdateContext context(std::move(request));

  context.InvokeErrorCallback(PropertyUpdateResult::kResourceNotFound);
  EXPECT_TRUE(callback_invoked_);
  EXPECT_FALSE(response_received_.IsSuccess());
  EXPECT_EQ(response_received_.GetErrorType(),
            PropertyUpdateResult::kResourceNotFound);
}

TEST_F(PropertyUpdateContextTest, CallbackInvocationLayerCallbackRequest) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value_callback = std::make_shared<MockAnimaXValueCallback>();
  auto callback = CreateTestCallback();

  auto request = LayerCallbackRequest::Make(
      LayerPropertyType::kTransformScale, std::move(key_path),
      std::move(value_callback), std::move(callback));

  PropertyUpdateContext context(std::move(request));

  context.InvokeErrorCallback(PropertyUpdateResult::kPropertyNotImplemented);
  EXPECT_TRUE(callback_invoked_);
  EXPECT_FALSE(response_received_.IsSuccess());
  EXPECT_EQ(response_received_.GetErrorType(),
            PropertyUpdateResult::kPropertyNotImplemented);
}
