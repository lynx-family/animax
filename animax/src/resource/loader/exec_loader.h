// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_LOADER_EXEC_LOADER_H_
#define ANIMAX_SRC_RESOURCE_LOADER_EXEC_LOADER_H_

#include <cassert>
#include <memory>

#include "base/include/fml/memory/ref_ptr.h"
#include "include/resource/loader.h"
#include "include/resource/loader_error.h"
#include "src/base/log/log.h"
#include "src/base/thread/task_runner.h"

namespace lynx {
namespace animax {

// Create an ExecLoader using a specified executor.
// An executor is any callable conforming to the signature
// `void(executor)(void(task)())`. ExecLoader does not perform null checks on
// executors.
template <typename In, typename Out, typename Exec>
class ExecLoader : public Loader<In, Out> {
 public:
  ExecLoader(typename Loader<In, Out>::Ptr loader, Exec exec)
      : loader_(std::move(loader)), exec_(std::move(exec)) {}

  ~ExecLoader() override = default;
  void Load(In in, typename Loader<In, Out>::CallbackType callback) override {
    exec_([in = std::move(in), callback = std::move(callback),
           weak_self = ExecLoader::weak_from_this()]() mutable {
      auto self = std::static_pointer_cast<ExecLoader>(weak_self.lock());
      if (self) {
        self->loader_->Load(std::move(in), [callback = std::move(callback),
                                            weak_self](auto out,
                                                       auto error) mutable {
          if (auto self =
                  std::static_pointer_cast<ExecLoader>(weak_self.lock());
              self) {
            self->exec_([callback = std::move(callback), out = std::move(out),
                         error = std::move(error)]() mutable {
              DCHECK(!!callback);
              if (callback) {
                // A nullptr callback indicates that the callback has been
                // called more than once. Subsequent calls will be ignored.
                callback(std::move(out), std::move(error));
              }
              // callback will be destroyed after this line.
            });
          }
        });
      }
    });
  }

 private:
  typename Loader<In, Out>::Ptr loader_;
  Exec exec_;
};

template <typename In, typename Out>
class TaskRunnerExecLoader : public Loader<In, Out> {
 public:
  ~TaskRunnerExecLoader() override = default;
  explicit TaskRunnerExecLoader(
      const fml::RefPtr<fml::TaskRunner>& (*get_task_runner)(),
      typename Loader<In, Out>::Ptr loader)
      : get_task_runner_{get_task_runner}, loader_(std::move(loader)) {
    DCHECK(get_task_runner_);
    DCHECK(loader_);
  }

  void Load(In in, typename Loader<In, Out>::CallbackType callback) override {
    fml::TaskRunner::RunNowOrPostTask(
        (*get_task_runner_)(),
        [in = std::move(in), callback = std::move(callback), loader = loader_,
         get_task_runner = get_task_runner_]() mutable {
          loader->Load(
              std::move(in), [callback = std::move(callback), get_task_runner](
                                 auto out, auto error) mutable {
                fml::TaskRunner::RunNowOrPostTask(
                    (*get_task_runner)(),
                    [callback = std::move(callback), out = std::move(out),
                     error = std::move(error)]() mutable {
                      DCHECK(!!callback);
                      if (callback) {
                        // A nullptr callback indicates that the callback has
                        // been called more than once. Subsequent calls will be
                        // ignored.
                        callback(std::move(out), std::move(error));
                      }
                      // callback will be destroyed after this line.
                    });
              });
        });
  }

 private:
  const fml::RefPtr<fml::TaskRunner>& (*get_task_runner_)() = nullptr;
  typename Loader<In, Out>::Ptr loader_;
};

template <typename In, typename Out, typename Exec>
static typename Loader<In, Out>::Ptr MakeExecLoader(
    std::shared_ptr<Loader<In, Out>> loader, Exec&& exec) {
  return std::shared_ptr<Loader<In, Out>>{
      new ExecLoader<In, Out, Exec>{loader, std::forward<Exec>(exec)}};
}

template <typename In, typename Out>
typename std::shared_ptr<Loader<In, Out>> ResourceThreadExecLoader(
    const std::shared_ptr<Loader<In, Out>>& loader) {
  return TaskRunnerExecLoader<In, Out>::template Make<
      TaskRunnerExecLoader<In, Out>>(&GetAnimaXResourceThread, loader);
}

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_LOADER_EXEC_LOADER_H_
