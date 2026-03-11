// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_TASK_RUNNER_H_
#define ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_TASK_RUNNER_H_

#include "base/include/closure.h"
#include "third_party/binding/napi/shim/shim_napi.h"

namespace lynx {
namespace animax {

class NapiTaskRunner {
 public:
  using Task = lynx::base::closure;
  /**
   * Creates and returns a move-only NapiTaskRunner instance.
   *
   * The new instance of NapiTaskRunner is bounded to the provided Napi
   * environment. The NapiTaskRunner enables tasks to be posted and executed on
   * the JavaScript thread associated with the given Napi environment.
   *
   * The NapiTaskRunner does not guarantee the execution of posted tasks, as the
   * Napi::Env might be closed, making task posting ineffective.
   *
   * @param env The Napi::Env environment where the NapiTaskRunner will post
   * tasks.
   *
   */
  NapiTaskRunner(Napi::Env env);

  /**
   * Creates a NapiTaskRunner that is not bounded to any Napi::Env.
   * PostTask on such NapiTaskRunner is noop.
   */
  NapiTaskRunner();

  NapiTaskRunner(NapiTaskRunner&&);
  NapiTaskRunner& operator=(NapiTaskRunner&&);

  /**
   * Posts a task to be executed on the JavaScript thread associated with the
   * NapiTaskRunner instance.
   *
   * You do not have to keep the NapiTaskRunner not released before the task
   * is actually invoked.
   *
   * @param task The Task to be posted.
   * @return true if the task was successfully posted; false if it could not be
   * posted, possibly due to the Napi runtime of the JS thread being closed.
   */
  bool PostTask(Task task);
  ~NapiTaskRunner();

 private:
  struct NapiTaskRunnerImpl;
  std::unique_ptr<NapiTaskRunnerImpl> impl_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_TASK_RUNNER_H_
