// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <charconv>
#include <functional>
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "include/resource/loader.h"
#include "include/resource/loader_error.h"
#include "src/resource/loader/exec_loader.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/loader/pipe_loader.h"

using namespace lynx::animax;
using namespace ::testing;

class IntToStringLoader : public Loader<int, std::string> {
 public:
  ~IntToStringLoader() override = default;
  void Load(int i, CallbackType callback) override {
    callback(std::to_string(i), LoaderError{});
  }
};

class StringToIntLoader : public Loader<std::string, int> {
 public:
  ~StringToIntLoader() override = default;
  void Load(std::string i, CallbackType callback) override {
    const auto* first = i.data();
    const auto* last = first + i.size();
    int value;
    auto [ptr, ec] = std::from_chars(first, last, value);
    if (ec == std::errc{}) {
      callback(value, LoaderError{});
    } else {
      callback(0, LoaderError{.code = kInvalidArgument});
    }
  }
};

TEST(LoaderExample, CreateLoader) {
  // In this test, we demonstrate the recommended usage of std::shared_ptr for
  // managing the lifecycle of loaders. Loaders should be exclusively managed
  // via std::shared_ptr to ensure proper memory management and avoid leaks.

  // There are several ways to instantiate loaders with std::shared_ptr:

  // 1. Using std::make_shared simplifies the code and is generally safer and
  // more efficient than using new directly.
  auto int_to_string_loader0 = std::make_shared<IntToStringLoader>();

  // 2. Direct usage of new with std::shared_ptr is also possible.
  auto int_to_string_loader1 =
      std::shared_ptr<IntToStringLoader>(new IntToStringLoader());

  // 3. The static Make method facilitates instantiation and automatically casts
  // the loader to its base type, Loader<int, std::string>.
  auto int_to_string_loader2 =
      Loader<int, std::string>::Make<IntToStringLoader>();
  auto int_to_string_loader3 = IntToStringLoader::Make<IntToStringLoader>();

  // 4. Explicit casting using std::static_pointer_cast can be used as needed.
  auto int_to_string_loader4 =
      std::static_pointer_cast<Loader<int, std::string>>(
          std::make_shared<IntToStringLoader>());
}

TEST(LoaderExample, CreateLambdaLoader) {
  // This test illustrates the creation of loaders using lambda expressions for
  // simple loading operations. Lambda loaders are ideal when the loading logic
  // is straightforward and can be encapsulated in a single function.
  auto int_to_string_lambda_loader0 =
      MakeLambdaLoader<int, std::string>([](auto i, auto callback) {
        // Convert the integer to string and invoke the callback with no error.
        callback(std::to_string(i), LoaderError{});
      });

  // Beyond lambda functions, loaders can be crafted from any callable object
  // that matches the required signature. Here, we use std::function to define a
  // loader's behavior explicitly.
  auto int_to_string_func =
      std::function<void(int, Loader<int, std::string>::CallbackType)>(
          [](auto i, auto callback) {
            callback(std::to_string(i), LoaderError{});
          });
  auto int_to_string_lambda_loader1 =
      MakeLambdaLoader<int, std::string>(std::move(int_to_string_func));

  // Loaders can also accommodate move-only callable objects, useful for cases
  // where copying is expensive or not feasible.
  auto int_to_string_move_only_func =
      lynx::base::MoveOnlyClosure<void, int,
                                  Loader<int, std::string>::CallbackType>(
          [](auto i, auto callback) {
            callback(std::to_string(i), LoaderError{});
          });
  auto int_to_string_lambda_loader2 = MakeLambdaLoader<int, std::string>(
      std::move(int_to_string_move_only_func));
}

TEST(LoaderExample, MoreLambdaLoader) {
  // Demonstrates the use of a lambda loader to create more sophisticated
  // behaviors in the loading process, such as logging. This example logs the
  // input and error of another loader.
  auto input = std::string{""};
  auto error = LoaderError{};

  auto string_to_int_loader = StringToIntLoader::Make<StringToIntLoader>();
  // The lambda loader here wraps the string_to_int_loader to capture and log
  // input and any errors.
  auto logged_string_to_int_loader = MakeLambdaLoader<std::string, int>(
      [&input, &error, string_to_int_loader](auto i, auto callback) {
        input = i;  // Log input.
        string_to_int_loader->Load(
            i, [&error, callback = std::move(callback)](auto i, auto e) {
              error = e;       // Log any errors.
              callback(i, e);  // Continue the loader chain.
            });
      });

  // Test the loader with a non-numeric input to trigger an error and log it.
  logged_string_to_int_loader->Load("Not a number.", [](auto i, auto e) {});
  EXPECT_EQ("Not a number.",
            input);  // Check that the input was logged correctly.
  EXPECT_EQ(kInvalidArgument,
            error.code);  // Verify that the correct error code was logged.
}

TEST(LoaderExample, PipeLoader) {
  // In this test, we demonstrate how to chain loaders together using a piping
  // mechanism. This allows the output of one loader to seamlessly become the
  // input of the next. Piping is effective when you have loaders with
  // compatible input and output types and want to create complex data
  // transformation workflows.

  // First, we instantiate two basic loaders: one that converts integers to
  // strings and another that converts strings back to integers.
  auto int_to_string_loader = IntToStringLoader::Make<IntToStringLoader>();
  auto string_to_int_loader = StringToIntLoader::Make<StringToIntLoader>();

  // Here, we pipe the two loaders together. The piped loader takes an integer,
  // converts it to a string, and then converts that string back to an integer.
  auto int_to_int_loader = int_to_string_loader | string_to_int_loader;

  // We test the piped loader with a known value to verify it processes the data
  // correctly and handles errors appropriately.
  int_to_int_loader->Load(42, [](auto i, auto error) {
    EXPECT_FALSE(!!error);  // Expect no error.
    EXPECT_EQ(
        42, i);  // The output should be the same as the input after conversion.
  });

  // Loaders can also be piped with lambda-based loaders, which provide a
  // flexible way to insert custom logic into the loader chain.
  auto int_to_string_lambda_loader =
      MakeLambdaLoader<int, std::string>([](auto i, auto callback) {
        callback(std::to_string(i),
                 LoaderError{});  // Convert integer to string and pass along
                                  // with no error.
      });

  // Here, we combine the integer-to-integer loader with a new lambda loader
  // that converts integers back to strings.
  auto int_to_string_loader2 = int_to_int_loader | int_to_string_lambda_loader;

  // We test the final loader in the chain with the same integer to verify it
  // now converts integers to their string representations correctly.
  int_to_string_loader2->Load(42, [](auto i, auto error) {
    EXPECT_FALSE(!!error);  // Expect no error.
    EXPECT_EQ("42", i);     // The output should be the string "42".
  });
}
