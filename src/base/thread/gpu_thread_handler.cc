// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/thread/gpu_thread_handler.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

GPUThreadHolder::GPUThreadHolder() {
  GPUThreadPool::IndexedRunner index_runner =
      GPUThreadPool::Instance().AcquireThreadRunner();
  runner_ = index_runner.runner;
  index_ = index_runner.index;
  DCHECK(runner_);
}

GPUThreadHolder::GPUThreadHolder(size_t index) {
  GPUThreadPool::IndexedRunner index_runner =
      GPUThreadPool::Instance().AcquireThreadRunner(index);
  runner_ = index_runner.runner;
  index_ = index_runner.index;
  DCHECK(runner_);
}

GPUThreadHolder::~GPUThreadHolder() {
  if (runner_) {
    GPUThreadPool::Instance().ReleaseThreadRunner(index_);
  }
}

const RunnerPtr& GPUThreadHolder::Get() { return runner_; }

void GPUThreadHolder::PostTask(base::closure task) {
  DCHECK(runner_);
  runner_->PostTask(std::move(task));
}

void GPUThreadHolder::PostEmergencyTask(base::closure task) {
  DCHECK(runner_);
  runner_->PostEmergencyTask(std::move(task));
}

GPUThreadHolder::GPUThreadHolder(GPUThreadHolder&& other) noexcept
    : runner_(std::move(other.runner_)), index_(std::move(other.index_)) {}

GPUThreadHolder& GPUThreadHolder::operator=(GPUThreadHolder&& other) noexcept {
  if (this != &other) {
    if (runner_) {
      GPUThreadPool::Instance().ReleaseThreadRunner(index_);
    }
    runner_ = std::move(other.runner_);
    index_ = std::move(other.index_);
  }
  return *this;
}

}  // namespace animax
}  // namespace lynx
