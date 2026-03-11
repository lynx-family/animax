// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/base/thread/gpu_thread_handler.h"

#include <atomic>
#include <chrono>
#include <future>

#include "gtest/gtest.h"
#include "src/base/thread/gpu_thread_pool.h"
using namespace lynx::animax;

TEST(AnimaXGPUThreadHolder, ConstructionAndDestruction) {
  auto holder = new GPUThreadHolder();
  EXPECT_NE(holder, nullptr);
  EXPECT_NE(holder->Get(), nullptr);
  delete holder;
}

TEST(AnimaXGPUThreadHolder, MultiConstruction) {
  auto holder1 = GPUThreadHolder();
  auto holder2 = GPUThreadHolder();
  EXPECT_NE(holder1.Get(), holder2.Get());
}

TEST(AnimaXGPUThreadHolder, MoveConstruction) {
  auto holder = GPUThreadHolder();
  EXPECT_NE(holder.Get(), nullptr);
  RunnerPtr runner = holder.Get();
  auto holder_move = GPUThreadHolder(std::move(holder));
  EXPECT_EQ(holder.Get(), nullptr);
  EXPECT_EQ(holder_move.Get(), runner);
  auto holder_equal = GPUThreadHolder();
  EXPECT_NE(holder_equal.Get(), runner);
  holder_equal = std::move(holder_move);
  EXPECT_EQ(holder_move.Get(), nullptr);
  EXPECT_EQ(holder_equal.Get(), runner);
}

TEST(AnimaXGPUThreadHolder, MoveConstructionSelf) {
  GPUThreadHolder holder;
  EXPECT_NE(holder.Get(), nullptr);
  RunnerPtr original_runner = holder.Get();
  holder = std::move(holder);
  EXPECT_NE(holder.Get(), nullptr) << "Self-move should be a no-op.";
  EXPECT_EQ(holder.Get(), original_runner)
      << "Runner should be the same after self-move.";
}

TEST(AnimaXGPUThreadHolder, MoveToNullAndNullDestruction) {
  auto holder1 = GPUThreadHolder();
  EXPECT_NE(holder1.Get(), nullptr);
  auto holder2 = std::move(holder1);
  EXPECT_EQ(holder1.Get(), nullptr);
  auto holder3 = new GPUThreadHolder();
  EXPECT_NE(holder3->Get(), nullptr);
  holder1 = std::move(*holder3);
  EXPECT_EQ(holder3->Get(), nullptr);
  delete holder3;
}

TEST(AnimaXGPUThreadHolder, PostTask) {
  auto holder = GPUThreadHolder();
  EXPECT_NE(holder.Get(), nullptr);
  std::atomic<bool> task_was_executed(false);
  std::promise<void> task_completion_promise;
  std::future<void> task_completion_future =
      task_completion_promise.get_future();
  holder.PostTask([&task_was_executed,
                   promise = std::move(task_completion_promise)]() mutable {
    task_was_executed.store(true, std::memory_order_relaxed);
    promise.set_value();
  });
  auto future_status = task_completion_future.wait_for(std::chrono::seconds(1));
  EXPECT_EQ(future_status, std::future_status::ready);
  EXPECT_TRUE(task_was_executed.load());
}

TEST(AnimaXGPUThreadHolder, PostEmergencyTask) {
  auto holder = GPUThreadHolder();
  EXPECT_NE(holder.Get(), nullptr);
  std::atomic<bool> task_was_executed(false);
  std::promise<void> task_completion_promise;
  std::future<void> task_completion_future =
      task_completion_promise.get_future();
  holder.PostEmergencyTask(
      [&task_was_executed,
       promise = std::move(task_completion_promise)]() mutable {
        task_was_executed.store(true, std::memory_order_relaxed);
        promise.set_value();
      });
  auto future_status = task_completion_future.wait_for(std::chrono::seconds(1));
  EXPECT_EQ(future_status, std::future_status::ready);
  EXPECT_TRUE(task_was_executed.load());
}
