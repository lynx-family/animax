// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/android/animax_property_callback_android.h"

#include "base/include/platform/android/jni_convert_helper.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXPropertyCallback_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXPropertyCallback_register_jni.h"
#include "src/base/util/android/java_only_array.h"

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXPropertyCallback(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

void AnimaXPropertyCallbackAndroid::OnPropertyCallback(
    const PropertyUpdateResponse& response) {
  JNIEnv* env = base::android::AttachCurrentThread();
  lynx::animax::android::JavaOnlyArray error_message_array;
  auto error_messages = response.GetUpdateMessages();
  for (auto message : error_messages) {
    error_message_array.PushString(message);
  }
  Java_AnimaXPropertyCallback_onPropertyCallback(
      env, callback_.Get(), error_message_array.jni_object());
}

}  // namespace animax
}  // namespace lynx
