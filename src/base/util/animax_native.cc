// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/animax_native.h"

#include "base/include/no_destructor.h"

namespace lynx {
namespace animax {

AnimaXNative& AnimaXNative::Instance() {
  static base::NoDestructor<AnimaXNative> instance;
  return *instance;
}

void AnimaXNative::RegisterALogFunction(ALogFunction logger_function) {
  log_function_ = logger_function;
}

ALogFunction AnimaXNative::GetALogFunction() { return log_function_; }

}  // namespace animax
}  // namespace lynx
