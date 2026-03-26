// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/base/thread/gpu_thread_pool.h"

#include <algorithm>
#include <random>

#include "gtest/gtest.h"
#include "src/base/thread/task_runner.h"
using namespace lynx::animax;

TEST(AnimaXGPUThreadPool, Init) {
  EXPECT_NE(nullptr, &GPUThreadPool::Instance());
  auto runner = GPUThreadPool::Instance().AcquireThreadRunner();
  EXPECT_NE(nullptr, runner.runner);
  GPUThreadPool::Instance().ReleaseThreadRunner(runner.index);
}

TEST(AnimaXGPUThreadPool, SimpleCase) {
  auto runner1 = GPUThreadPool::Instance().AcquireThreadRunner();
  auto runner2 = GPUThreadPool::Instance().AcquireThreadRunner();
  EXPECT_NE(runner1.index, runner2.index);
  GPUThreadPool::Instance().ReleaseThreadRunner(runner1.index);
  auto runner3 = GPUThreadPool::Instance().AcquireThreadRunner();
  EXPECT_EQ(runner1.index, runner3.index);
  GPUThreadPool::Instance().ReleaseThreadRunner(runner1.index);
  // Extra Release. Won't do anything.
  GPUThreadPool::Instance().ReleaseThreadRunner(runner3.index);
  // Won't do anything.
  GPUThreadPool::Instance().ReleaseThreadRunner(-1);
  GPUThreadPool::Instance().ReleaseThreadRunner(runner2.index);
}

TEST(AnimaXGPUThreadPool, LargeCase) {
  std::vector<GPUThreadPool::IndexedRunner> runners(100);
  std::unordered_map<size_t, int> count_map;
  for (auto& runner : runners) {
    runner = GPUThreadPool::Instance().AcquireThreadRunner();
    if (count_map.count(runner.index)) {
      count_map[runner.index]++;
    } else {
      count_map[runner.index] = 1;
    }
  }
  int max_count = -1;
  int min_count = 300;
  for (auto runner : runners) {
    min_count = std::min(min_count, count_map[runner.index]);
    max_count = std::max(max_count, count_map[runner.index]);
    GPUThreadPool::Instance().ReleaseThreadRunner(runner.index);
  }
  EXPECT_TRUE(max_count - min_count <= 1);
}

TEST(AnimaXGPUThreadPool, RandomRelease) {
  std::vector<GPUThreadPool::IndexedRunner> runners(100);
  std::unordered_map<size_t, int> count_map;
  for (auto& runner : runners) {
    runner = GPUThreadPool::Instance().AcquireThreadRunner();
    if (count_map.count(runner.index)) {
      count_map[runner.index]++;
    } else {
      count_map[runner.index] = 1;
    }
  }
  int max_count = -1;
  for (auto runner : runners) {
    max_count = std::max(max_count, count_map[runner.index]);
  }
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(runners.begin(), runners.end(), g);
  for (int i = 0; i < 20; ++i) {
    GPUThreadPool::Instance().ReleaseThreadRunner(runners[i].index);
    count_map[runners[i].index]--;
  }
  for (int i = 0; i < 20; ++i) {
    runners[i] = GPUThreadPool::Instance().AcquireThreadRunner();
    count_map[runners[i].index]++;
  }
  int after_max_count = -1;
  for (auto runner : runners) {
    after_max_count = std::max(after_max_count, count_map[runner.index]);
    GPUThreadPool::Instance().ReleaseThreadRunner(runner.index);
  }
  EXPECT_EQ(after_max_count, max_count);
}

TEST(AnimaXGPUThreadPool, SingleThread) {
  GPUThreadPool::Instance().AcquireThreadRunner(0);
  auto runner2 = GPUThreadPool::Instance().AcquireThreadRunner();
  EXPECT_NE(runner2.index, 0);
  GPUThreadPool::Instance().ReleaseThreadRunner(0);
  GPUThreadPool::Instance().ReleaseThreadRunner(runner2.index);
}
