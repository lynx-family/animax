// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/property/property_update_response.h"

#include <algorithm>
#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace lynx {
namespace animax {

class PropertyUpdateResponseTest : public ::testing::Test {};

TEST_F(PropertyUpdateResponseTest, DefaultConstructor) {
  PropertyUpdateResponse response;
  EXPECT_TRUE(response.IsSuccess());
  EXPECT_TRUE(response.GetUpdateMessages().empty());
  EXPECT_EQ(response.GetErrorType(), PropertyUpdateResult::kSuccess);
}

TEST_F(PropertyUpdateResponseTest, ConstructorWithPreCheckError) {
  PropertyUpdateResponse response(PropertyUpdateResult::kEnvInvalid);
  EXPECT_FALSE(response.IsSuccess());
  EXPECT_EQ(response.GetErrorType(), PropertyUpdateResult::kEnvInvalid);

  auto messages = response.GetUpdateMessages();
  ASSERT_EQ(messages.size(), 1u);
  EXPECT_EQ(messages[0], "pre-check failure, error_code:" +
                             std::to_string(static_cast<int16_t>(
                                 PropertyUpdateResult::kEnvInvalid)));
}

TEST_F(PropertyUpdateResponseTest, AddTargetErrors) {
  PropertyUpdateResponse response;
  response.AddErrorType("target1", PropertyUpdateResult::kPropertyNull);
  response.AddErrorType("target2", PropertyUpdateResult::kValueInvalid);

  EXPECT_FALSE(response.IsSuccess());

  // The returned error type will be the first one in the map's iteration
  // order.
  PropertyUpdateResult error_type = response.GetErrorType();
  EXPECT_TRUE(error_type == PropertyUpdateResult::kPropertyNull ||
              error_type == PropertyUpdateResult::kValueInvalid);

  auto messages = response.GetUpdateMessages();
  ASSERT_EQ(messages.size(), 2u);

  std::string msg1 =
      "<target1> , error_code:" +
      std::to_string(static_cast<int16_t>(PropertyUpdateResult::kPropertyNull));
  std::string msg2 =
      "<target2> , error_code:" +
      std::to_string(static_cast<int16_t>(PropertyUpdateResult::kValueInvalid));

  // The order of messages from an unordered_map is not guaranteed.
  EXPECT_TRUE(std::find(messages.begin(), messages.end(), msg1) !=
              messages.end());
  EXPECT_TRUE(std::find(messages.begin(), messages.end(), msg2) !=
              messages.end());
}

TEST_F(PropertyUpdateResponseTest, SetPreCheckError) {
  PropertyUpdateResponse response;
  response.AddErrorType("target1", PropertyUpdateResult::kPropertyNull);
  response.SetErrorType(PropertyUpdateResult::kResourceNotFound);

  EXPECT_FALSE(response.IsSuccess());
  EXPECT_EQ(response.GetErrorType(), PropertyUpdateResult::kResourceNotFound);

  auto messages = response.GetUpdateMessages();
  ASSERT_EQ(messages.size(), 1u);
  EXPECT_EQ(messages[0], "pre-check failure, error_code:" +
                             std::to_string(static_cast<int16_t>(
                                 PropertyUpdateResult::kResourceNotFound)));
}

TEST_F(PropertyUpdateResponseTest, IsSuccessLogic) {
  PropertyUpdateResponse response;
  EXPECT_TRUE(response.IsSuccess());

  response.SetErrorType(PropertyUpdateResult::kSuccess);
  EXPECT_TRUE(response.IsSuccess());

  response.SetErrorType(PropertyUpdateResult::kKeyPathMismatched);
  EXPECT_FALSE(response.IsSuccess());

  PropertyUpdateResponse response2;
  response2.AddErrorType("target", PropertyUpdateResult::kPropertyNotFound);
  EXPECT_FALSE(response2.IsSuccess());
}

TEST_F(PropertyUpdateResponseTest, GetErrorTypeLogic) {
  PropertyUpdateResponse response;
  EXPECT_EQ(response.GetErrorType(), PropertyUpdateResult::kSuccess);

  response.AddErrorType("target1", PropertyUpdateResult::kValueInvalid);
  EXPECT_EQ(response.GetErrorType(), PropertyUpdateResult::kValueInvalid);

  response.AddErrorType("target2", PropertyUpdateResult::kPropertyNull);
  PropertyUpdateResult first_error = response.GetErrorType();
  EXPECT_TRUE(first_error == PropertyUpdateResult::kValueInvalid ||
              first_error == PropertyUpdateResult::kPropertyNull);

  response.SetErrorType(PropertyUpdateResult::kEnvInvalid);
  EXPECT_EQ(response.GetErrorType(), PropertyUpdateResult::kEnvInvalid);
}

TEST_F(PropertyUpdateResponseTest, GetUpdateMessagesEmpty) {
  PropertyUpdateResponse response;
  auto messages = response.GetUpdateMessages();
  EXPECT_TRUE(messages.empty());
}

}  // namespace animax
}  // namespace lynx
