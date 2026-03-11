// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <emscripten/emscripten.h>

#include "base/include/fml/macros.h"
#include "base/include/fml/message_loop.h"
#include "base/include/fml/message_loop_impl.h"
#include "base/include/fml/task_runner_delegate.h"
#include "base/include/no_destructor.h"
#include "src/base/log/log.h"
#include "src/base/thread/gpu_thread_handler.h"
#include "src/base/thread/task_runner.h"
#include "src/base/thread/thread_assert.h"

namespace lynx {
namespace fml {
class MessageLoopAnimaXWeb : public MessageLoopImpl {
 public:
  MessageLoopAnimaXWeb() = default;
  ~MessageLoopAnimaXWeb() override { Terminate(); }
  void Run() override { running_ = true; }
  void Terminate() override { running_ = false; }
  void WakeUp(fml::TimePoint time_point) override {}
  // FML_FRIEND_MAKE_REF_COUNTED(MessageLoopAnimaXWeb);
  // FML_FRIEND_REF_COUNTED_THREAD_SAFE(MessageLoopAnimaXWeb);
  bool running_ = false;
};
RefPtr<MessageLoopImpl> MessageLoopImpl::Create(void* platform_loop) {
  return AdoptRef<MessageLoopImpl>(new MessageLoopAnimaXWeb());
}
}  // namespace fml

namespace animax {
class TaskRunnerWebDelegate : public fml::TaskRunnerDelegate {
 public:
  void PostTask(lynx::base::closure task) {
    emscripten_async_call(
        [](void* arg) {
          auto* t = static_cast<base::closure*>(arg);
          (*t)();
          delete t;
        },
        new base::closure(std::move(task)), 0);
  }

  void PostTaskForTime(lynx::base::closure task, int64_t target_time) {
    DCHECK(false) << "PostTaskForTime is not supported";
  }

  void PostDelayedTask(lynx::base::closure task, int64_t delay) {
    DCHECK(false) << "PostDelayedTask is not supported";
  }

  bool RunsTasksOnCurrentThread() { return true; }
};

class MainThreadTaskRunner {
 public:
  static const fml::RefPtr<fml::TaskRunner>& Get() {
    static base::NoDestructor<Holder> holder_;
    std::call_once(holder_->init_flag, [&]() {
      static base::NoDestructor<TaskRunnerWebDelegate> delegate;
      auto& loop = fml::MessageLoop::EnsureInitializedForCurrentThread();
      holder_->task_runner = loop.GetTaskRunner();
      holder_->task_runner->SetDelegate(&(*delegate));
    });
    return holder_->task_runner;
  }

 private:
  struct Holder {
    fml::RefPtr<fml::TaskRunner> task_runner;
    std::once_flag init_flag;
  };
};

const fml::RefPtr<lynx::fml::TaskRunner>& GetAnimaXMainThread() {
  return MainThreadTaskRunner::Get();
}

std::shared_ptr<GPUThreadHolder> GetAnimaXGPUThreadHolder(
    bool enable_multi_gpu_thread) {
  struct HolderWrapper {
    std::shared_ptr<GPUThreadHolder> holder;
    explicit HolderWrapper(std::shared_ptr<GPUThreadHolder> h)
        : holder(std::move(h)) {}
  };
  static base::NoDestructor<HolderWrapper> wrapper(
      std::make_shared<GPUThreadHolder>());
  return wrapper->holder;
}

const fml::RefPtr<lynx::fml::TaskRunner>& GetAnimaXResourceThread() {
  return MainThreadTaskRunner::Get();
}

GPUThreadHolder::GPUThreadHolder()
    : runner_(MainThreadTaskRunner::Get()), index_(0) {}

GPUThreadHolder::GPUThreadHolder(size_t index)
    : runner_(MainThreadTaskRunner::Get()), index_(index) {}

GPUThreadHolder::~GPUThreadHolder() = default;

GPUThreadHolder::GPUThreadHolder(GPUThreadHolder&& other) noexcept = default;

GPUThreadHolder& GPUThreadHolder::operator=(GPUThreadHolder&& other) noexcept =
    default;

const RunnerPtr& GPUThreadHolder::Get() { return runner_; }

void GPUThreadHolder::PostTask(base::closure task) {
  DCHECK(runner_);
  runner_->PostTask(std::move(task));
}

void GPUThreadHolder::PostEmergencyTask(base::closure task) {
  PostTask(std::move(task));
}

}  // namespace animax
}  // namespace lynx
