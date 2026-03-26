// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_LOADER_LAMBDA_LOADER_H_
#define ANIMAX_SRC_RESOURCE_LOADER_LAMBDA_LOADER_H_

#include <memory>

#include "include/resource/loader.h"

namespace lynx {
namespace animax {

template <typename In, typename Out, typename F>
class LambdaLoader : public Loader<In, Out> {
 public:
  explicit LambdaLoader(F&& f) : f_(std::move(f)) {}
  ~LambdaLoader() override = default;
  void Load(In in, typename Loader<In, Out>::CallbackType callback) override {
    f_(std::move(in), std::move(callback));
  }

 private:
  F f_;
};

template <typename In, typename Out, typename F>
std::shared_ptr<Loader<In, Out>> MakeLambdaLoader(F&& f) {
  return Loader<In, Out>::template Make<LambdaLoader<In, Out, F>>(
      std::forward<F>(f));
}
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_LOADER_LAMBDA_LOADER_H_
