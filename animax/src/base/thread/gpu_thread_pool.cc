// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/base/thread/gpu_thread_pool.h"

#include "base/include/no_destructor.h"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"

static constexpr const char* kAnimaXGPUThreadName = "Animax_GPU";
static constexpr lynx::fml::Thread::ThreadPriority kAnimaXGPUThreadPriority =
    lynx::fml::Thread::ThreadPriority::HIGH;

namespace lynx {
namespace animax {

GPUThreadPool& GPUThreadPool::Instance() {
  static lynx::base::NoDestructor<GPUThreadPool> instance;
  return *instance;
}

GPUThreadPool::IndexedRunner GPUThreadPool::AcquireThreadRunner() {
  std::lock_guard<std::mutex> lock(mutex_);
  GPUThreadPool::IndexedRunner indexed_runner;
  size_t ref_count;
  BucketIterator bucket_iterator = buckets_.begin();
  if (buckets_.front().ref_count != 0 &&
      runner_contexts_.size() < max_pool_size_) {
    indexed_runner = CreateNewThread();
    ref_count = 1;
  } else {
    ref_count = buckets_.begin()->ref_count + 1;
    indexed_runner =
        DetachThread(buckets_.begin()->thread_indexes.front(), bucket_iterator);
  }
  AttachThread(indexed_runner.index, ref_count, std::next(bucket_iterator));
  DeleteBucketIfNeeded(bucket_iterator);
  return indexed_runner;
}

GPUThreadPool::IndexedRunner GPUThreadPool::AcquireThreadRunner(size_t index) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!IsIndexValid(index)) {
    ANIMAX_LOGI(
        "GPU Thread Pool Acquire thread with runner Fail. Thread do not exist.")
    return {static_cast<size_t>(-1), nullptr};
  }
  BucketIterator bucket = *runner_contexts_[index].bucket;
  size_t ref_count = bucket->ref_count;
  IndexedRunner indexed_runner = DetachThread(index, bucket);
  AttachThread(indexed_runner.index, ref_count + 1, std::next(bucket));
  DeleteBucketIfNeeded(bucket);
  return indexed_runner;
}

void GPUThreadPool::ReleaseThreadRunner(size_t index) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!IsIndexValid(index)) {
    ANIMAX_LOGI(
        "GPU Thread Pool Acquire thread with runner Fail. Thread do not exist.")
    return;
  }
  BucketIterator bucket = *runner_contexts_[index].bucket;
  size_t ref_count = bucket->ref_count;
  if (ref_count <= 0) {
    ANIMAX_LOGI("GPU Thread Pool Release thread Fail. Thread released before.")
    return;
  }
  IndexedRunner indexed_runner = DetachThread(index, bucket);
  AttachThread(indexed_runner.index, ref_count - 1, bucket);
  DeleteBucketIfNeeded(bucket);
}

GPUThreadPool::GPUThreadPool() {
  size_t cpu_cores = std::thread::hardware_concurrency();
  initial_pool_size_ = 1;
  max_pool_size_ = cpu_cores * 2;
  runner_contexts_.reserve(max_pool_size_);

  ANIMAX_LOGI("Initializing GPU Thread pool with size: "
              << initial_pool_size_ << ", max size: " << max_pool_size_)

  for (size_t i = 0; i < initial_pool_size_; ++i) {
    IndexedRunner indexed_runner = CreateNewThread();
    AttachThread(indexed_runner.index, 0, buckets_.begin());
  }
}

GPUThreadPool::IndexedRunner GPUThreadPool::CreateNewThread() {
  std::string thread_name =
      runner_contexts_.empty()
          ? kAnimaXGPUThreadName
          : kAnimaXGPUThreadName + std::to_string(runner_contexts_.size());
  auto thread = std::make_unique<lynx::fml::Thread>(
      lynx::fml::Thread::ThreadConfig(thread_name, kAnimaXGPUThreadPriority));
  thread->GetTaskRunner()->PostTask(
      []() { ThreadAssert::Init(ThreadAssert::Type::kGPU); });
  IndexedRunner indexed_runner{runner_contexts_.size(),
                               thread->GetTaskRunner()};
  runner_contexts_.push_back(
      RunnerContext{std::nullopt, std::nullopt, std::move(thread)});
  return indexed_runner;
}

void GPUThreadPool::AttachThread(size_t index, size_t ref_count,
                                 const BucketIterator& origin_iterator) {
  BucketIterator bucket;
  if (origin_iterator != buckets_.begin() &&
      std::prev(origin_iterator)->ref_count == ref_count) {
    bucket = std::prev(origin_iterator);
  } else if (origin_iterator != buckets_.end() &&
             origin_iterator->ref_count == ref_count) {
    bucket = origin_iterator;
  } else {
    bucket = buckets_.emplace(origin_iterator, ref_count);
  }
  bucket->thread_indexes.emplace_front(index);
  runner_contexts_[index].index = bucket->thread_indexes.begin();
  runner_contexts_[index].bucket = bucket;
}

GPUThreadPool::IndexedRunner GPUThreadPool::DetachThread(
    size_t index, BucketIterator bucket_iterator) {
  bucket_iterator->thread_indexes.erase(*runner_contexts_[index].index);
  return {index, runner_contexts_[index].thread->GetTaskRunner()};
}

void GPUThreadPool::DeleteBucketIfNeeded(
    GPUThreadPool::BucketIterator bucket_iterator) {
  if (bucket_iterator->thread_indexes.size() == 0) {
    buckets_.erase(bucket_iterator);
  }
}

GPUThreadPool::Bucket::Bucket(size_t ref_count) : ref_count(ref_count) {}
}  // namespace animax
}  // namespace lynx
