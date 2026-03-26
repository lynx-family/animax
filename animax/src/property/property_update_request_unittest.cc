// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/property/property_update_request.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

// Mock implementation of AnimaXValueCallback for testing
class MockAnimaXValueCallback : public AnimaXValueCallback {
 public:
  std::unique_ptr<Value> GetValue(const Value* original_value,
                                  const AnimaXFrameInfo& frame_info) override {
    return nullptr;
  }
};

class PropertyUpdateRequestTest : public ::testing::Test {
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

TEST_F(PropertyUpdateRequestTest, LayerStaticRequestMake) {
  auto key_path = std::make_unique<AnimaXKeyPath>(
      std::vector<std::string>{"layer1", "transform"});
  auto value = std::make_unique<AnimaXValueParam>(42.5);
  auto callback = CreateTestCallback();

  auto request = LayerStaticRequest::Make(LayerPropertyType::kTransformRotation,
                                          std::move(key_path), std::move(value),
                                          std::move(callback));

  EXPECT_NE(request, nullptr);
  EXPECT_EQ(request->GetRequestType(),
            PropertyUpdateRequest::Type::kLayerStatic);
  EXPECT_EQ(request->GetLayerType(), LayerPropertyType::kTransformRotation);
}

TEST_F(PropertyUpdateRequestTest, LayerStaticRequestGetKeyPath) {
  std::vector<std::string> path_keys = {"layer1", "transform", "rotation"};
  auto key_path = std::make_unique<AnimaXKeyPath>(path_keys);
  auto value = std::make_unique<AnimaXValueParam>(90.0);
  auto callback = CreateTestCallback();

  auto request = LayerStaticRequest::Make(LayerPropertyType::kTransformRotation,
                                          std::move(key_path), std::move(value),
                                          std::move(callback));

  const auto& retrieved_path = request->GetKeyPath();
  EXPECT_EQ(retrieved_path.GetKeys(), path_keys);
}

TEST_F(PropertyUpdateRequestTest, LayerStaticRequestGetValue) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value = std::make_unique<AnimaXValueParam>(std::string("test_value"));
  auto callback = CreateTestCallback();

  auto request = LayerStaticRequest::Make(LayerPropertyType::kTextValue,
                                          std::move(key_path), std::move(value),
                                          std::move(callback));

  const auto& retrieved_value = request->GetValue();
  EXPECT_TRUE(retrieved_value.IsString());
  EXPECT_EQ(retrieved_value.GetStringValue(), "test_value");
}

TEST_F(PropertyUpdateRequestTest, LayerStaticRequestInvokeCallback) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value = std::make_unique<AnimaXValueParam>(100.0);
  auto callback = CreateTestCallback();

  auto request =
      LayerStaticRequest::Make(LayerPropertyType::kOpacity, std::move(key_path),
                               std::move(value), std::move(callback));

  PropertyUpdateResponse test_response;

  EXPECT_FALSE(callback_invoked_);
  request->InvokeCallback(test_response);
  EXPECT_TRUE(callback_invoked_);
  EXPECT_TRUE(response_received_.IsSuccess());
}

TEST_F(PropertyUpdateRequestTest, LayerCallbackRequestMake) {
  auto key_path = std::make_unique<AnimaXKeyPath>(
      std::vector<std::string>{"layer1", "color"});
  auto value_callback = std::make_shared<MockAnimaXValueCallback>();
  auto callback = CreateTestCallback();

  auto request = LayerCallbackRequest::Make(
      LayerPropertyType::kColor, std::move(key_path), std::move(value_callback),
      std::move(callback));

  EXPECT_NE(request, nullptr);
  EXPECT_EQ(request->GetRequestType(),
            PropertyUpdateRequest::Type::kLayerCallback);
  EXPECT_EQ(request->GetLayerType(), LayerPropertyType::kColor);
}

TEST_F(PropertyUpdateRequestTest, LayerCallbackRequestGetKeyPath) {
  std::vector<std::string> path_keys = {"layer1", "stroke", "color"};
  auto key_path = std::make_unique<AnimaXKeyPath>(path_keys);
  auto value_callback = std::make_shared<MockAnimaXValueCallback>();
  auto callback = CreateTestCallback();

  auto request = LayerCallbackRequest::Make(
      LayerPropertyType::kStrokeColor, std::move(key_path),
      std::move(value_callback), std::move(callback));

  const auto& retrieved_path = request->GetKeyPath();
  EXPECT_EQ(retrieved_path.GetKeys(), path_keys);
}

TEST_F(PropertyUpdateRequestTest, LayerCallbackRequestGetValueCallback) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value_callback = std::make_shared<MockAnimaXValueCallback>();
  auto callback = CreateTestCallback();

  auto request = LayerCallbackRequest::Make(
      LayerPropertyType::kTransformPosition, std::move(key_path),
      value_callback, std::move(callback));

  auto retrieved_callback = request->GetValueCallback();
  EXPECT_EQ(retrieved_callback, value_callback);
  EXPECT_NE(retrieved_callback, nullptr);
}

TEST_F(PropertyUpdateRequestTest, LayerCallbackRequestInvokeCallback) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value_callback = std::make_shared<MockAnimaXValueCallback>();
  auto callback = CreateTestCallback();

  auto request = LayerCallbackRequest::Make(
      LayerPropertyType::kTransformScale, std::move(key_path),
      std::move(value_callback), std::move(callback));

  PropertyUpdateResponse test_response;

  EXPECT_FALSE(callback_invoked_);
  request->InvokeCallback(test_response);
  EXPECT_TRUE(callback_invoked_);
}

TEST_F(PropertyUpdateRequestTest, ResourceUpdateRequestMake) {
  std::string resource_id = "image_resource_123";
  auto value =
      std::make_unique<AnimaXValueParam>(std::string("new_image_path.png"));
  auto callback = CreateTestCallback();

  auto request = ResourceUpdateRequest::Make(
      ResourcePropertyType::kImageFileName, resource_id, std::move(value),
      std::move(callback));

  EXPECT_NE(request, nullptr);
  EXPECT_EQ(request->GetRequestType(), PropertyUpdateRequest::Type::kResource);
  EXPECT_EQ(request->GetResourceType(), ResourcePropertyType::kImageFileName);
}

TEST_F(PropertyUpdateRequestTest, ResourceUpdateRequestGetResourceId) {
  std::string resource_id = "font_resource_456";
  auto value = std::make_unique<AnimaXValueParam>(std::string("new_font.ttf"));
  auto callback = CreateTestCallback();

  auto request =
      ResourceUpdateRequest::Make(ResourcePropertyType::kFontPath, resource_id,
                                  std::move(value), std::move(callback));

  EXPECT_EQ(request->GetResourceId(), "font_resource_456");
}

TEST_F(PropertyUpdateRequestTest, ResourceUpdateRequestGetValue) {
  std::string resource_id = "image_resource_789";
  auto value = std::make_unique<AnimaXValueParam>(1920.0);
  auto callback = CreateTestCallback();

  auto request = ResourceUpdateRequest::Make(ResourcePropertyType::kImageWidth,
                                             resource_id, std::move(value),
                                             std::move(callback));

  const auto& retrieved_value = request->GetValue();
  EXPECT_TRUE(retrieved_value.IsNumber());
  EXPECT_NEAR(retrieved_value.GetNumberValue(), 1920.0, 0.001);
}

TEST_F(PropertyUpdateRequestTest, ResourceUpdateRequestInvokeCallback) {
  std::string resource_id = "test_resource";
  auto value = std::make_unique<AnimaXValueParam>(123.456);
  auto callback = CreateTestCallback();

  auto request = ResourceUpdateRequest::Make(ResourcePropertyType::kVideoWidth,
                                             resource_id, std::move(value),
                                             std::move(callback));

  PropertyUpdateResponse test_response;

  EXPECT_FALSE(callback_invoked_);
  request->InvokeCallback(test_response);
  EXPECT_TRUE(callback_invoked_);
  EXPECT_TRUE(response_received_.IsSuccess());
}

TEST_F(PropertyUpdateRequestTest, CallbackWithNullCallback) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer1"});
  auto value = std::make_unique<AnimaXValueParam>(42.0);

  auto request =
      LayerStaticRequest::Make(LayerPropertyType::kOpacity, std::move(key_path),
                               std::move(value), nullptr);

  PropertyUpdateResponse test_response;
  request->InvokeCallback(test_response);
  EXPECT_FALSE(callback_invoked_);
}

TEST_F(PropertyUpdateRequestTest, LayerStaticRequestMultiplePropertyTypes) {
  struct TestCase {
    LayerPropertyType property_type;
    std::string description;
  };

  std::vector<TestCase> test_cases = {
      {LayerPropertyType::kTransformRotation, "rotation"},
      {LayerPropertyType::kTransformScale, "scale"},
      {LayerPropertyType::kTransformPosition, "position"},
      {LayerPropertyType::kOpacity, "opacity"},
      {LayerPropertyType::kColor, "color"},
      {LayerPropertyType::kStrokeColor, "stroke_color"},
      {LayerPropertyType::kTextValue, "text_value"}};

  for (const auto& test_case : test_cases) {
    auto key_path =
        std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer"});
    auto value = std::make_unique<AnimaXValueParam>(1.0);
    auto callback = CreateTestCallback();

    auto request =
        LayerStaticRequest::Make(test_case.property_type, std::move(key_path),
                                 std::move(value), std::move(callback));

    EXPECT_EQ(request->GetLayerType(), test_case.property_type)
        << "Failed for property type: " << test_case.description;
  }
}

TEST_F(PropertyUpdateRequestTest, ResourceUpdateRequestMultipleResourceTypes) {
  struct TestCase {
    ResourcePropertyType resource_type;
    std::string description;
  };

  std::vector<TestCase> test_cases = {
      {ResourcePropertyType::kImageFileName, "image_filename"},
      {ResourcePropertyType::kFontPath, "font_path"},
      {ResourcePropertyType::kImageWidth, "image_width"},
      {ResourcePropertyType::kVideoHeight, "video_height"}};

  for (const auto& test_case : test_cases) {
    auto value = std::make_unique<AnimaXValueParam>(std::string("test_value"));
    auto callback = CreateTestCallback();

    auto request =
        ResourceUpdateRequest::Make(test_case.resource_type, "test_resource_id",
                                    std::move(value), std::move(callback));

    EXPECT_EQ(request->GetResourceType(), test_case.resource_type)
        << "Failed for resource type: " << test_case.description;
  }
}

TEST_F(PropertyUpdateRequestTest, LayerStaticRequestWithComplexKeyPath) {
  std::vector<std::string> complex_path = {"root_layer", "**",   "group", "*",
                                           "shape",      "fill", "color"};
  auto key_path = std::make_unique<AnimaXKeyPath>(complex_path);
  auto value =
      std::make_unique<AnimaXValueParam>(static_cast<int32_t>(0xFFFF0000));
  auto callback = CreateTestCallback();

  auto request =
      LayerStaticRequest::Make(LayerPropertyType::kColor, std::move(key_path),
                               std::move(value), std::move(callback));

  const auto& retrieved_path = request->GetKeyPath();
  EXPECT_EQ(retrieved_path.GetKeys(), complex_path);
}

TEST_F(PropertyUpdateRequestTest, LayerStaticRequestWithPointValue) {
  auto key_path = std::make_unique<AnimaXKeyPath>(
      std::vector<std::string>{"layer", "transform"});
  auto value = std::make_unique<AnimaXValueParam>(100.5, 200.7);
  auto callback = CreateTestCallback();

  auto request = LayerStaticRequest::Make(LayerPropertyType::kTransformPosition,
                                          std::move(key_path), std::move(value),
                                          std::move(callback));

  const auto& retrieved_value = request->GetValue();
  EXPECT_TRUE(retrieved_value.IsCoordinate());
  EXPECT_DOUBLE_EQ(retrieved_value.GetX(), 100.5);
  EXPECT_NEAR(retrieved_value.GetY(), 200.7, 0.001);
}

TEST_F(PropertyUpdateRequestTest, LayerStaticRequestWithBooleanValue) {
  auto key_path =
      std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"layer"});
  auto value = std::make_unique<AnimaXValueParam>(true);
  auto callback = CreateTestCallback();

  auto request = LayerStaticRequest::Make(LayerPropertyType::kVisibility,
                                          std::move(key_path), std::move(value),
                                          std::move(callback));

  const auto& retrieved_value = request->GetValue();
  EXPECT_TRUE(retrieved_value.IsBoolean());
  EXPECT_TRUE(retrieved_value.GetBooleanValue());
}

TEST_F(PropertyUpdateRequestTest, MockValueCallbackTest) {
  auto mock_callback = std::make_shared<MockAnimaXValueCallback>();

  AnimaXFrameInfo frame_info(0.0f, 100.0f, 50.0f, 0.5f, 0.5f, 0.5f);
  auto result = mock_callback->GetValue(nullptr, frame_info);
  EXPECT_EQ(result, nullptr);
}
