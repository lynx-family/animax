// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/android/java_only_array.h"

#include "base/include/platform/android/jni_convert_helper.h"
#include "base/include/platform/android/jni_utils.h"
#include "platform/android/animax_android/src/main/jni/gen/JavaOnlyArray_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/JavaOnlyArray_register_jni.h"
#include "src/base/util/android/java_only_map.h"

namespace lynx {
namespace animax {
namespace android {

JavaOnlyArray::JavaOnlyArray() {
  JNIEnv* env = base::android::AttachCurrentThread();
  jni_object_.Reset(env, Java_JavaOnlyArray_create(env).Get());
}

void JavaOnlyArray::PushArray(JavaOnlyArray* value) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_JavaOnlyArray_pushArray(env, jni_object_.Get(), value->jni_object());
}

void JavaOnlyArray::PushString(const std::string& value) {
  JNIEnv* env = base::android::AttachCurrentThread();

  static const int api_level = android_get_device_api_level();
  // Emoji will make App crash when use `NewStringUTF` API in Android 5.x
  if (api_level < __ANDROID_API_M__) {
    base::android::ScopedLocalJavaRef<jbyteArray> jni_value =
        base::android::JNIConvertHelper::ConvertToJNIByteArray(env, value);
    Java_JavaOnlyArray_pushByteArrayAsString(env, jni_object_.Get(),
                                             jni_value.Get());
  } else {
    base::android::ScopedLocalJavaRef<jstring> jni_value =
        base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, value);
    Java_JavaOnlyArray_pushString(env, jni_object_.Get(), jni_value.Get());
  }
}

int32_t JavaOnlyArray::JavaOnlyArrayGetSize(JNIEnv* env, jobject array) {
  return Java_JavaOnlyArray_size(env, array);
}

ReadableType JavaOnlyArray::JavaOnlyArrayGetTypeAtIndex(JNIEnv* env,
                                                        jobject array,
                                                        int32_t index) {
  return static_cast<ReadableType>(
      Java_JavaOnlyArray_getTypeIndex(env, array, index));
}

std::string JavaOnlyArray::JavaOnlyArrayGetStringAtIndex(JNIEnv* env,
                                                         jobject array,
                                                         int32_t index) {
  return base::android::JNIConvertHelper::ConvertToString(
      env, Java_JavaOnlyArray_getString(env, array, index).Get());
}

}  // namespace android
}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForJavaOnlyArray(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
