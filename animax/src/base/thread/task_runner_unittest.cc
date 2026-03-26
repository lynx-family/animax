// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/base/thread/task_runner.h"

#include "gtest/gtest.h"
using namespace lynx::animax;

TEST(AnimaXTaskRunner, ThreadMain) {
  auto runner = GetAnimaXMainThread();
  EXPECT_NE(nullptr, runner);
  auto runner2 = GetAnimaXMainThread();
  EXPECT_EQ(runner, runner2);
}

TEST(AnimaXTaskRunner, ThreadResource) {
  auto runner = GetAnimaXResourceThread();
  EXPECT_NE(nullptr, runner);
  auto runner2 = GetAnimaXResourceThread();
  EXPECT_EQ(runner, runner2);
}

TEST(AnimaXTaskRunner, ThreadGPU) {
  auto runner = GetAnimaXGPUThreadHolder(false);
  EXPECT_NE(nullptr, runner->Get());
  auto runner2 = GetAnimaXGPUThreadHolder(false);
  EXPECT_EQ(runner->Get(), runner2->Get());
  auto runner3 = GetAnimaXGPUThreadHolder(true);
  EXPECT_NE(runner->Get(), runner3->Get());
}
