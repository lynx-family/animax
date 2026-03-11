// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/loader/pipe_loader.h"

#include <memory>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/resource/loader.h"
#include "include/resource/loader_error.h"

using namespace lynx::animax;
using namespace testing;

using StrToIntLoader = Loader<std::string, int>;
using IntToStrLoader = Loader<int, std::string>;

class StoiMockLoader : public StrToIntLoader {
 public:
  ~StoiMockLoader() override = default;
  MOCK_METHOD(void, Load, (std::string, CallbackType), (override));
};

class ItosMockLoader : public IntToStrLoader {
 public:
  ~ItosMockLoader() override = default;
  MOCK_METHOD(void, Load, (int, CallbackType), (override));
};

TEST(PipeLoaderTest, PipeLoaderFirstLoaderFail) {
  auto l1 = StrToIntLoader::Make<StoiMockLoader>();
  auto mock_l1 = std::static_pointer_cast<StoiMockLoader>(l1);
  auto l2 = IntToStrLoader::Make<ItosMockLoader>();
  auto mock_l2 = std::static_pointer_cast<ItosMockLoader>(l2);
  auto p = l1 | l2;
  const auto err_msg = std::string{"error!"};
  EXPECT_NE(nullptr, p);
  EXPECT_CALL(*mock_l1, Load(_, _))
      .WillOnce([err_msg](auto input, auto callback) {
        callback(0, LoaderError{.code = LoaderErrorCode::kUserDefined,
                                .message = err_msg});
      });
  bool callback_called = false;
  p->Load("", [&callback_called, err_msg](auto res, auto error) {
    callback_called = true;
    EXPECT_TRUE(!!error);
    EXPECT_EQ(err_msg, error.message);
  });
}

TEST(PipeLoaderTest, PipeLoaderSecondLoaderFail) {
  auto l1 = StrToIntLoader::Make<StoiMockLoader>();
  auto mock_l1 = std::static_pointer_cast<StoiMockLoader>(l1);
  auto l2 = IntToStrLoader::Make<ItosMockLoader>();
  auto mock_l2 = std::static_pointer_cast<ItosMockLoader>(l2);
  auto p = l1 | l2;
  EXPECT_NE(nullptr, p);
  const auto err_msg = std::string{"error!"};
  EXPECT_CALL(*mock_l1, Load(_, _)).WillOnce([](auto input, auto callback) {
    callback(0, LoaderError{});
  });
  EXPECT_CALL(*mock_l2, Load(_, _))
      .WillOnce([err_msg](auto input, auto callback) {
        callback("", LoaderError{.code = LoaderErrorCode::kUserDefined,
                                 .message = err_msg});
      });
  bool callback_called = false;
  p->Load("", [&callback_called, err_msg](auto res, auto error) {
    callback_called = true;
    EXPECT_TRUE(!!error);
    EXPECT_EQ(err_msg, error.message);
  });
}

TEST(PipeLoaderTest, PipeLoaderErrorPropagation1) {
  auto l1 = StrToIntLoader::Make<StoiMockLoader>();
  auto mock_l1 = std::static_pointer_cast<StoiMockLoader>(l1);
  auto l2 = IntToStrLoader::Make<ItosMockLoader>();
  auto mock_l2 = std::static_pointer_cast<ItosMockLoader>(l2);
  auto l3 = StrToIntLoader::Make<StoiMockLoader>();
  auto mock_l3 = std::static_pointer_cast<StoiMockLoader>(l3);
  auto p = l1 | l2 | l3;
  EXPECT_NE(nullptr, p);

  const auto err_msg = std::string{"error!"};
  // If the error happens in the l1 loader, then l2 and l3 should be skipped.
  // The error will be propagted to the original callback
  EXPECT_CALL(*mock_l1, Load(_, _))
      .WillOnce([err_msg](auto input, auto callback) {
        callback(0, LoaderError{kUserDefined, err_msg});
      });
  EXPECT_CALL(*mock_l2, Load(_, _)).Times(0);
  EXPECT_CALL(*mock_l3, Load(_, _)).Times(0);
  bool callback_called = false;
  p->Load("", [&callback_called, err_msg](auto res, auto error) {
    callback_called = true;
    EXPECT_TRUE(!!error);
    EXPECT_EQ(err_msg, error.message);
  });
}

TEST(PipeLoaderTest, PipeLoaderErrorPropagation2) {
  auto l1 = StrToIntLoader::Make<StoiMockLoader>();
  auto mock_l1 = std::static_pointer_cast<StoiMockLoader>(l1);
  auto l2 = IntToStrLoader::Make<ItosMockLoader>();
  auto mock_l2 = std::static_pointer_cast<ItosMockLoader>(l2);
  auto l3 = StrToIntLoader::Make<StoiMockLoader>();
  auto mock_l3 = std::static_pointer_cast<StoiMockLoader>(l3);
  auto p = l1 | l2 | l3;
  EXPECT_NE(nullptr, p);

  const auto err_msg = std::string{"error!"};
  const auto first_result = 1;
  EXPECT_CALL(*mock_l1, Load(_, _))
      .WillOnce([first_result](auto input, auto callback) {
        callback(first_result, LoaderError{});
      });
  // If the error happens in the l2 loader, then l3 should be skipped.
  // The error will be propagted to the original callback
  EXPECT_CALL(*mock_l2, Load(first_result, _))
      .WillOnce([err_msg](auto input, auto callback) {
        callback("", LoaderError{kUserDefined, err_msg});
      });
  EXPECT_CALL(*mock_l3, Load(_, _)).Times(0);
  bool callback_called = false;
  p->Load("", [&callback_called, err_msg](auto res, auto error) {
    callback_called = true;
    EXPECT_TRUE(!!error);
    EXPECT_EQ(err_msg, error.message);
  });
}
