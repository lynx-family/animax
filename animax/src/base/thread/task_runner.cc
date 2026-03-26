// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/thread/task_runner.h"

#include "base/include/fml/message_loop.h"
#include "base/include/no_destructor.h"
#include "src/base/thread/thread_assert.h"

static constexpr const char* kAnimaXMainThreadName = "Animax_Main";
static constexpr lynx::fml::Thread::ThreadPriority kAnimaXMainThreadPriority =
    lynx::fml::Thread::ThreadPriority::HIGH;
static constexpr const char* kAnimaXResourceThreadName = "Animax_Resource";
static constexpr lynx::fml::Thread::ThreadPriority
    kAnimaXResourceThreadPriority = lynx::fml::Thread::ThreadPriority::NORMAL;

namespace lynx {
namespace animax {

const fml::RefPtr<lynx::fml::TaskRunner>& GetAnimaXMainThread() {
  static lynx::base::NoDestructor<lynx::fml::Thread> animax_main_thread{
      lynx::fml::Thread::ThreadConfig(kAnimaXMainThreadName,
                                      kAnimaXMainThreadPriority)};
  static std::once_flag main_task_runner_init_flag;
  std::call_once(main_task_runner_init_flag, []() {
    animax_main_thread->GetTaskRunner()->PostTask([]() {
      lynx::animax::ThreadAssert::Init(lynx::animax::ThreadAssert::Type::kMain);
    });
  });

  return animax_main_thread->GetTaskRunner();
}

std::shared_ptr<GPUThreadHolder> GetAnimaXGPUThreadHolder(
    bool enable_multi_gpu_thread) {
  if (!enable_multi_gpu_thread) {
    return std::make_shared<GPUThreadHolder>(0);
  }
  return std::make_shared<GPUThreadHolder>();
}

const fml::RefPtr<lynx::fml::TaskRunner>& GetAnimaXResourceThread() {
  static lynx::base::NoDestructor<lynx::fml::Thread> animax_thread{
      lynx::fml::Thread::ThreadConfig(kAnimaXResourceThreadName,
                                      kAnimaXResourceThreadPriority)};
  static std::once_flag resource_task_runner_init_flag;
  std::call_once(resource_task_runner_init_flag, []() {
    animax_thread->GetTaskRunner()->PostTask([]() {
      lynx::animax::ThreadAssert::Init(
          lynx::animax::ThreadAssert::Type::kResource);
    });
  });

  return animax_thread->GetTaskRunner();
}

}  // namespace animax
}  // namespace lynx
