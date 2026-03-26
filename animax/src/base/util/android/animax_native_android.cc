// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "platform/android/animax_android/src/main/jni/gen/AnimaX_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaX_register_jni.h"
#include "src/base/util/animax_native.h"

static void RegisterLogger(JNIEnv* env, jobject jcaller, jlong logger_ptr) {
  lynx::animax::ALogFunction log_func =
      reinterpret_cast<lynx::animax::ALogFunction>(logger_ptr);
  lynx::animax::AnimaXNative::Instance().RegisterALogFunction(log_func);
}

namespace animax {
namespace jni {
bool RegisterJNIForAnimaX(JNIEnv* env) { return RegisterNativesImpl(env); }
}  // namespace jni
}  // namespace animax
