// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/android/jni_helper.h"

#include "base/include/platform/android/jni_utils.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {
namespace android {

bool CheckException(JNIEnv *env) {
  std::string error_msg;
  bool has_no_exception = lynx::base::android::CheckException(env, error_msg);
  if (!has_no_exception) {
    ANIMAX_LOGE("JNI exception found: " << error_msg);
  }
  return has_no_exception;
}
}  // namespace android
}  // namespace animax
}  // namespace lynx
