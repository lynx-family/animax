// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <jni.h>

#include "base/include/no_destructor.h"
#include "src/base/log/log.h"
#include "src/layer/textra/text_helper_textra.h"

extern "C" __attribute__((visibility("default"))) int32_t JNI_OnLoad(JavaVM *vm,
                                                                     void *) {
  ANIMAX_LOGI("animax textra JNI_OnLoad");
  static lynx::base::NoDestructor<lynx::animax::TextHelperTextra> impl;
  lynx::animax::TextHelper::RegisterImpl(&(*impl));

  return JNI_VERSION_1_6;
}
