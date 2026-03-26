// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_RESOURCE_LOADER_H_
#define ANIMAX_INCLUDE_RESOURCE_LOADER_H_

#include <memory>
#include <type_traits>

#include "base/include/closure.h"
#include "include/base/macros.h"
#include "include/resource/loader_error.h"

namespace lynx {
namespace animax {

template <typename In, typename Out>
class ANIMAX_EXPORT Loader
    : public std::enable_shared_from_this<Loader<In, Out>> {
  static_assert(std::is_default_constructible_v<In> &&
                    std::is_default_constructible_v<Out>,
                "In and Out type must be default constructible");
  static_assert(std::is_move_constructible_v<In> &&
                    std::is_move_constructible_v<Out>,
                "In and Out type must be move constructible");

 public:
  using CallbackType = base::MoveOnlyClosure<void, Out, LoaderError>;
  using Ptr = typename std::shared_ptr<Loader<In, Out>>;
  using RequestType = In;
  using ResponseType = Out;

  template <typename T, typename... Args>
  static Ptr Make(Args... args) {
    return Ptr{new T{std::forward<Args>(args)...}};
  }

  virtual ~Loader() = default;
  virtual void Load(In, CallbackType) = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_RESOURCE_LOADER_H_
