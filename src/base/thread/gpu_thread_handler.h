// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_THREAD_GPU_THREAD_HANDLER_H_
#define ANIMAX_SRC_BASE_THREAD_GPU_THREAD_HANDLER_H_

#include "src/base/thread/gpu_thread_pool.h"

namespace lynx {
namespace animax {
// RAII wrapper for GPU thread management
class GPUThreadHolder {
 public:
  GPUThreadHolder();
  explicit GPUThreadHolder(size_t index);
  ~GPUThreadHolder();

  void PostTask(base::closure task);
  void PostEmergencyTask(base::closure task);
  const RunnerPtr& Get();

  GPUThreadHolder(const GPUThreadHolder&) = delete;
  GPUThreadHolder& operator=(const GPUThreadHolder&) = delete;
  GPUThreadHolder(GPUThreadHolder&& other) noexcept;
  GPUThreadHolder& operator=(GPUThreadHolder&& other) noexcept;

 private:
  RunnerPtr runner_;
  size_t index_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_THREAD_GPU_THREAD_HANDLER_H_
