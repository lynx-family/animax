// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_ANDROID_JNI_HELPER_H_
#define ANIMAX_SRC_BASE_UTIL_ANDROID_JNI_HELPER_H_

#include <jni.h>
namespace lynx {
namespace animax {
namespace android {

bool CheckException(JNIEnv* env);

}
}  // namespace animax
}  // namespace lynx
#endif  // ANIMAX_SRC_BASE_UTIL_ANDROID_JNI_HELPER_H_
