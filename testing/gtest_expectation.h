// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_TESTING_GTEST_EXPECTATION_H_
#define ANIMAX_TESTING_GTEST_EXPECTATION_H_

#include <gtest/gtest.h>

#include <future>
#include <memory>

class GTestExpectation {
 public:
  static std::shared_ptr<GTestExpectation> Make() {
    return std::shared_ptr<GTestExpectation>(new GTestExpectation());
  }

  void Fulfill() { promise_.set_value(); }

  bool Wait() {
    auto result = future_.wait_for(std::chrono::seconds(1));
    return result != std::future_status::ready;
  }

 private:
  GTestExpectation() = default;
  std::promise<void> promise_{};
  std::future<void> future_ = promise_.get_future();
};

#endif  // ANIMAX_TESTING_GTEST_EXPECTATION_H_
