// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_THREAD_TASK_RUNNER_H_
#define ANIMAX_SRC_BASE_THREAD_TASK_RUNNER_H_

#include "include/base/macros.h"
#include "src/base/thread/gpu_thread_handler.h"

namespace lynx {
namespace animax {

const RunnerPtr& GetAnimaXMainThread();

std::shared_ptr<GPUThreadHolder> GetAnimaXGPUThreadHolder(
    bool enable_multi_gpu_thread);
// Export the existing runner for the optional NAPI module's independent loads;
// this does not introduce another resource thread or change its lifetime.
ANIMAX_EXPORT const RunnerPtr& GetAnimaXResourceThread();

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_THREAD_TASK_RUNNER_H_
