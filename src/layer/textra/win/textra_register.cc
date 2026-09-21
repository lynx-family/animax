// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/win/textra_register.h"

#include <mutex>

#include "base/include/no_destructor.h"
#include "src/layer/textra/text_helper_textra.h"

namespace lynx {
namespace animax {

void RegisterAnimaXTextraWindows() {
  static std::once_flag once;
  std::call_once(once, [] {
    static base::NoDestructor<TextHelperTextra> impl;
    TextHelper::RegisterImpl(&(*impl));
  });
}

}  // namespace animax
}  // namespace lynx
