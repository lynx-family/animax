// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/android/animax_composition_callback_android.h"

#include "base/include/platform/android/jni_convert_helper.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXCompositionNativeCallback_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXCompositionNativeCallback_register_jni.h"

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXCompositionNativeCallback(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

void AnimaXCompositionCallbackAndroid::OnCompositionCallback(
    jobject composition, const std::string& error) {
  JNIEnv* env = base::android::AttachCurrentThread();
  auto error_msg =
      base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, error);
  Java_AnimaXCompositionNativeCallback_onCompositionCallback(
      env, callback_.Get(), composition, error_msg.Get());
}

}  // namespace animax
}  // namespace lynx
