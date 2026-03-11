// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/jsbridge/bindings/animax/napi_task_runner.h"

#include "base/include/no_destructor.h"
#include "src/base/log/log.h"
#include "third_party/binding/napi/napi_bridge.h"

#ifdef USE_PRIMJS_NAPI
#include "third_party/napi/include/primjs_napi_defines.h"
#endif

namespace lynx {
namespace animax {

namespace {
struct DataType {
  DataType(NapiTaskRunner::Task task)
      : task_{new base::MoveOnlyClosure<>{std::move(task)}} {}

  std::shared_ptr<base::MoveOnlyClosure<>> task_ = nullptr;
};

void Invoke(Napi::Env env, void* context, DataType data) {
  if (auto& task = data.task_; task) {
    (*task)();
  }
}

using FinalizerDataType = void;
void ThreadSafeFunctionFinalizer(Napi::Env env, void* finalizer_data, void*) {}

using ThreadSafeFunction = Napi::ThreadSafeFunction<void, DataType, &Invoke>;

std::unique_ptr<ThreadSafeFunction> CreateThreadSafeFunction(Napi::Env env) {
  return std::unique_ptr<ThreadSafeFunction>{ThreadSafeFunction::New(
      env, static_cast<void*>(nullptr), &ThreadSafeFunctionFinalizer,
      static_cast<void*>(nullptr))};
}
}  // namespace

struct NapiTaskRunner::NapiTaskRunnerImpl {
  std::unique_ptr<ThreadSafeFunction> thread_safe_function_ = nullptr;
};

NapiTaskRunner::NapiTaskRunner(Napi::Env env)
    : impl_{new NapiTaskRunnerImpl{CreateThreadSafeFunction(env)}} {}

NapiTaskRunner::NapiTaskRunner() = default;
NapiTaskRunner::NapiTaskRunner(NapiTaskRunner&&) = default;
NapiTaskRunner& NapiTaskRunner::operator=(NapiTaskRunner&&) = default;
NapiTaskRunner::~NapiTaskRunner() = default;

bool NapiTaskRunner::PostTask(Task task) {
  if (!impl_ || !impl_->thread_safe_function_) {
    return false;
  }
  const auto napi_status =
      impl_->thread_safe_function_->NonBlockingCall(std::move(task));
  if (napi_status == napi_ok) {
    return true;
  } else {
    ANIMAX_LOGE(
        "Failed to PostTask on Napi JS Thread, Napi Status: " << napi_status);
    return false;
  }
}

}  // namespace animax
}  // namespace lynx
