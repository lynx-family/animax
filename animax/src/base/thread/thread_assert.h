// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_THREAD_THREAD_ASSERT_H_
#define ANIMAX_SRC_BASE_THREAD_THREAD_ASSERT_H_

#include <cstdint>

#include "include/base/macros.h"

namespace lynx {
namespace animax {
class ThreadAssert final {
 public:
  enum class Type : uint8_t {
    kUnknown = 0,
    kGPU,
    kResource,
    kMain,
  };
  static void Init(const Type type);
  ANIMAX_EXPORT static void Assert(const Type type);
  static Type GetCurrent();

 private:
  static Type *Get();
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_THREAD_THREAD_ASSERT_H_
