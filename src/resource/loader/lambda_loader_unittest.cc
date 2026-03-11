// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/loader/lambda_loader.h"

#include <memory>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/resource/loader.h"
#include "include/resource/loader_error.h"

using namespace lynx::animax;
using namespace testing;

TEST(LambdaLoader, SimpleLambdaLoader) {
  bool is_exec_called = false;
  auto loader = MakeLambdaLoader<std::string, std::string>(
      [&is_exec_called](auto input, auto callback) {
        is_exec_called = true;
        callback(input, LoaderError{});
      });
  const auto input_str = std::string{"input"};
  loader->Load(input_str, [input_str](auto input, auto callback) {
    EXPECT_EQ(input, "input");
  });
  EXPECT_NE(nullptr, loader);
  EXPECT_TRUE(is_exec_called);
}
