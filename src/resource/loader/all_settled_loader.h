// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_LOADER_ALL_SETTLED_LOADER_H_
#define ANIMAX_SRC_RESOURCE_LOADER_ALL_SETTLED_LOADER_H_

#include <vector>

#include "include/resource/loader.h"
#include "include/resource/loader_error.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

template <typename Out>
struct AllSettledResponse {
  std::vector<Out> outs;
  std::vector<LoaderError> errors;
};

template <typename In, typename Out>
class AllSettledLoaderControl {
 public:
  AllSettledLoaderControl(
      int32_t counter,
      typename Loader<std::vector<In>, AllSettledResponse<Out>>::CallbackType
          callback)
      : counter_{counter}, callback_{std::move(callback)} {
    DCHECK(counter > 0);
    response_.outs.resize(counter);
    response_.errors.resize(counter);
  }
  void OnLoaderCallback(std::size_t index, Out out, LoaderError error) {
    response_.errors[index] = std::move(error);
    response_.outs[index] = std::move(out);
    if (--counter_ == 0) {
      callback_(std::move(response_), {});
    }
  }

 private:
  int32_t counter_{};
  typename Loader<std::vector<In>, AllSettledResponse<Out>>::CallbackType
      callback_{};
  AllSettledResponse<Out> response_{};
};

/**
 * @brief Facilitates the concurrent loading of multiple inputs, collecting
 * results and errors for each.
 *
 * AllSettledLoader manages the loading of a vector of inputs using an
 * underlying loader and aggregates the results and errors into an
 * AllSettledResponse. Each input is processed independently, and upon
 * completion of all, a callback is invoked with the aggregated results and
 * errors.
 *
 * Note:
 * This loader is not thread-safe. It relies on the assumption that the
 * underlying loader used for construction guarantees that its callback is
 * always invoked on the same thread. It is the responsibility of the
 * AllSettledLoader user to ensure that this thread safety condition is
 * maintained, as the AllSettledLoader itself does not manage thread safety. To
 * achieve thread safety for the underlying loader, it can be wrapped with an
 * ExecLoader to ensure that all callbacks are executed on the same thread.
 *
 * @tparam In Type of the input items to be loaded.
 * @tparam Out Type of the output items produced by the loader.
 *
 */
template <typename In, typename Out>
class AllSettledLoader
    : public Loader<std::vector<In>, AllSettledResponse<Out>> {
 public:
  explicit AllSettledLoader(typename Loader<In, Out>::Ptr loader)
      : loader_{std::move(loader)} {}
  ~AllSettledLoader() override = default;
  void Load(
      std::vector<In> in,
      typename Loader<std::vector<In>, AllSettledResponse<Out>>::CallbackType
          callback) override {
    if (in.empty()) {
      // If the in vector is empty, return an empty AllSettledResponse
      callback({}, {});
      return;
    }
    auto control = std::shared_ptr<AllSettledLoaderControl<In, Out>>{
        new AllSettledLoaderControl<In, Out>{static_cast<int32_t>(in.size()),
                                             std::move(callback)}};
    for (std::size_t i = 0; i < in.size(); ++i) {
      auto in_item = std::move(in[i]);
      loader_->Load(std::move(in_item), [index = i, control, called = false](
                                            Out out,
                                            LoaderError error) mutable {
        if (!called) {
          called = true;
          control->OnLoaderCallback(index, std::move(out), std::move(error));
        }
      });
    }
  }

 private:
  typename Loader<In, Out>::Ptr loader_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_LOADER_ALL_SETTLED_LOADER_H_
