// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_LOADER_PIPE_LOADER_H_
#define ANIMAX_SRC_RESOURCE_LOADER_PIPE_LOADER_H_

#include <memory>

#include "include/resource/loader.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

template <typename In1, typename Out1, typename Out2>
class PipeLoader : public Loader<In1, Out2> {
 public:
  PipeLoader() = default;
  PipeLoader(typename Loader<In1, Out1>::Ptr loader1,
             typename Loader<Out1, Out2>::Ptr loader2)
      : loader1_{std::move(loader1)}, loader2_{std::move(loader2)} {
    DCHECK(loader1_);
    DCHECK(loader2_);
  }

  void Load(In1 in1,
            typename Loader<In1, Out2>::CallbackType callback) override {
    loader1_->Load(std::move(in1),
                   [loader2 = loader2_, callback = std::move(callback)](
                       Out1 out1, LoaderError error) mutable {
                     if (error) {
                       callback(Out2{}, std::move(error));
                       return;
                     }
                     loader2->Load(std::move(out1), std::move(callback));
                   });
  }

  ~PipeLoader() override = default;

 private:
  typename Loader<In1, Out1>::Ptr loader1_{};
  typename Loader<Out1, Out2>::Ptr loader2_{};
};

template <typename In1, typename Out1, typename Out2>
std::shared_ptr<Loader<In1, Out2>> operator|(
    const std::shared_ptr<Loader<In1, Out1>>& loader1,
    const std::shared_ptr<Loader<Out1, Out2>>& loader2) {
  return Loader<In1, Out2>::template Make<PipeLoader<In1, Out1, Out2>>(loader1,
                                                                       loader2);
}

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_LOADER_PIPE_LOADER_H_
