// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_THREAD_GPU_THREAD_POOL_H_
#define ANIMAX_SRC_BASE_THREAD_GPU_THREAD_POOL_H_

#include <atomic>
#include <list>
#include <unordered_map>

#include "base/include/fml/thread.h"

namespace lynx {
namespace animax {

using ThreadPtr = std::unique_ptr<fml::Thread>;
using IndexIterator = std::list<size_t>::iterator;
using RunnerPtr = fml::RefPtr<fml::TaskRunner>;

// Thread pool implementation for managing GPU threads
class GPUThreadPool {
 public:
  struct IndexedRunner {
    size_t index;
    RunnerPtr runner;
  };

  static GPUThreadPool& Instance();
  IndexedRunner AcquireThreadRunner();
  IndexedRunner AcquireThreadRunner(size_t index);
  void ReleaseThreadRunner(size_t index);

  GPUThreadPool(const GPUThreadPool&) = delete;
  GPUThreadPool& operator=(const GPUThreadPool&) = delete;
  GPUThreadPool(GPUThreadPool&&) = delete;
  GPUThreadPool& operator=(GPUThreadPool&&) = delete;

 private:
  struct Bucket {
    std::list<size_t> thread_indexes;
    size_t ref_count;
    explicit Bucket(size_t ref_count);
  };

  using BucketIterator = std::list<Bucket>::iterator;

  friend class lynx::base::NoDestructor<GPUThreadPool>;
  GPUThreadPool();

  IndexedRunner CreateNewThread();
  void AttachThread(size_t index, size_t ref_count,
                    const BucketIterator& origin_iterator);
  IndexedRunner DetachThread(size_t index, BucketIterator bucket_iterator);
  void DeleteBucketIfNeeded(BucketIterator bucket_iterator);

  inline bool IsIndexValid(size_t index) {
    return index < runner_contexts_.size() &&
           runner_contexts_[index].index.has_value() &&
           runner_contexts_[index].bucket.has_value() &&
           runner_contexts_[index].thread != nullptr;
  }
  // clang-format off
  //       +---------------------------------------------------------------------------+
  //       |                          INTERNAL STATE                                   |
  //       |                                                                           |
  //       |   std::list<Bucket> buckets_;                                             |
  //       |   (Sorted by ref_count, conceptually)                                     |
  //       |                                                                           |
  //       |   Bucket (ref_count=0)    Bucket (ref_count=1)    Bucket (ref_count=N)    |
  //       |  +-------------------+   +-------------------+   +-------------------+    |
  //       |  | ref_count: 0      |   | ref_count: 1      |   | ref_count: N      |    |
  //       |  | thread_pool:      |   | thread_pool:      |   | thread_pool:      |    |
  //       |  | +---------------+ |   | +---------------+ |   | +---------------+ |    |
  //       |  | | ThreadPtr 0   | |   | | ThreadPtr 2   | |   | | ThreadPtr 3   | |    |
  //       |  | | ThreadPtr 1   | |   | +---------------+ |   | +---------------+ |    |
  //       |  | +---------------+ |   |                   |   |                   |    |
  //       |  +-------------------+   +-------------------+   +-------------------+    |
  //       +---------------------------------------------------------------------------+
  // clang-format on

  // The core container forall thread buckets, organized as a linked list. This
  // list maintains all active buckets, sorted in ascending order based on their
  // ref_count. This ordering is crucial, as it ensures the bucket with the
  // least-used threads is always at the front. The main workflow of this pool
  // is moving threads between these buckets. When a thread's reference count
  // changes, it is first removed from its current bucket using DetachThread.
  // After being detached, the thread is then added to a new bucket using
  // AttachThread. This new bucket corresponds to the thread's updated reference
  // count. If a bucket for the new reference count does not exist, AttachThread
  // creates it on-the-fly. This detach-and-attach mechanism ensures each thread
  // always resides in the bucket that reflects its current usage.
  std::list<Bucket> buckets_;
  struct RunnerContext {
    std::optional<BucketIterator> bucket;
    std::optional<IndexIterator> index;
    ThreadPtr thread;
  };
  std::vector<RunnerContext> runner_contexts_;
  std::mutex mutex_;
  size_t initial_pool_size_;
  size_t max_pool_size_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_THREAD_GPU_THREAD_POOL_H_
