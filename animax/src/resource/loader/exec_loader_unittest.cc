// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/loader/exec_loader.h"

#include <chrono>
#include <future>
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "include/resource/loader.h"
#include "include/resource/loader_error.h"
#include "src/base/thread/task_runner.h"
#include "src/base/thread/thread_assert.h"
#include "src/resource/loader/lambda_loader.h"

using namespace lynx::animax;
using namespace testing;

// ExecLoader does not perform null checks on executors.
TEST(ExecLoaderTest, NullExecLoader) {
  // Create a moved-from MoveOnlyClosure to simulate a null executor.
  auto exec =
      lynx::base::MoveOnlyClosure<void, lynx::base::MoveOnlyClosure<>>();
  auto inner_loader = MakeLambdaLoader<std::string, std::string>(
      [](auto in, auto cb) { cb({}, {.code = kInvalidLoader}); });
  // Construct an ExecLoader with the null-like executor.
  auto loader =
      MakeExecLoader<std::string, std::string>(inner_loader, std::move(exec));
  EXPECT_NE(nullptr, loader);  // Verify loader construction is successful.

  // Expect the application to exit when loading due to a null executor.
  EXPECT_EXIT(
      [loader]() {
        // Attempt to load with dummy data and expect an error callback due to
        // null executor.
        loader->Load("1", [](auto input, auto error) { EXPECT_TRUE(!!error); });
      }(),
      [](int) { return true; }, ".*");
}

// calling the callback more than once in debug mode will
// hit the assertion.
TEST(ExecLoaderTest, CallCallbackMoreThanOnce) {
  auto inner_loader =
      MakeLambdaLoader<std::string, std::string>([](auto in, auto cb) {
        // call callback more than once
        cb({}, {.code = kInvalidLoader});
        cb({}, {.code = kInvalidLoader});
      });
  // Construct an ExecLoader with the null-like executor.
  auto loader = MakeExecLoader<std::string, std::string>(
      inner_loader, [](auto task) { task(); });
  EXPECT_NE(nullptr, loader);  // Verify loader construction is successful.

  // Expect the application to exit
  EXPECT_EXIT(
      [loader]() {
        loader->Load("1", [](auto input, auto error) { EXPECT_TRUE(!!error); });
      }(),
      [](int) { return true; }, ".*");
}

TEST(ExecLoaderTest, Success) {
  int exec_called_time = 0;
  auto inner_loader = MakeLambdaLoader<std::string, std::string>(
      [](auto in, auto cb) { cb(std::string{"success!"}, {}); });
  auto loader = MakeExecLoader<std::string, std::string>(
      inner_loader, [&exec_called_time](auto task) mutable {
        ++exec_called_time;
        task();
      });

  EXPECT_NE(nullptr, loader);
  loader->Load("1", [](auto input, auto error) {
    EXPECT_FALSE(!!error);
    EXPECT_EQ("success!", input);
  });
  EXPECT_EQ(2, exec_called_time);
}

TEST(ExecLoaderTest, error) {
  int exec_called_time = 0;
  auto inner_loader =
      MakeLambdaLoader<std::string, std::string>([](auto in, auto cb) {
        cb(std::string{}, LoaderError{.code = kInvalidLoader});
      });
  auto loader = MakeExecLoader<std::string, std::string>(
      inner_loader, [&exec_called_time](auto task) mutable {
        ++exec_called_time;
        task();
      });

  EXPECT_NE(nullptr, loader);
  loader->Load("1", [](auto input, auto error) { EXPECT_TRUE(!!error); });
  EXPECT_EQ(2, exec_called_time);
}

TEST(ExecLoaderTest, ResourceThreadExecLoader) {
  auto promise = std::promise<void>{};
  auto future = promise.get_future();
  auto inner_loader =
      MakeLambdaLoader<std::string, std::string>([](auto in, auto cb) {
        EXPECT_EQ(ThreadAssert::Type::kResource, ThreadAssert::GetCurrent());
        cb(std::string{}, LoaderError{.code = kInvalidLoader});
      });
  auto loader =
      ResourceThreadExecLoader<std::string, std::string>(inner_loader);

  EXPECT_NE(nullptr, loader);
  loader->Load("1", [&promise](auto input, auto error) {
    EXPECT_EQ(ThreadAssert::Type::kResource, ThreadAssert::GetCurrent());
    EXPECT_TRUE(!!error);
    promise.set_value();
  });

  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(5)));
}
