// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "base/include/no_destructor.h"
#include "src/layer/textra/text_helper_textra.h"

extern "C" __attribute__((constructor)) void RegisterTextra() {
  static lynx::base::NoDestructor<lynx::animax::TextHelperTextra> impl;
  lynx::animax::TextHelper::RegisterImpl(&(*impl));
}
