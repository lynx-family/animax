// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/android/java_only_map.h"

#include <android/api-level.h>

#include <memory>
#include <utility>

#include "base/include/platform/android/jni_convert_helper.h"
#include "base/include/platform/android/jni_utils.h"
#include "platform/android/animax_android/src/main/jni/gen/JavaOnlyMap_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/JavaOnlyMap_register_jni.h"
#include "src/base/util/android/java_only_array.h"

namespace lynx {
namespace animax {
namespace android {

JavaOnlyMap::JavaOnlyMap() {
  JNIEnv* env = base::android::AttachCurrentThread();
  jni_object_.Reset(env, Java_JavaOnlyMap_create(env).Get());
}

void JavaOnlyMap::PushString(const std::string& key, const std::string& value) {
  PushString(key.c_str(), value.c_str());
}

void JavaOnlyMap::PushString(const char* key, const char* value) {
  JNIEnv* env = base::android::AttachCurrentThread();

  static const int api_level = android_get_device_api_level();
  // Emoji will make App crash when use `NewStringUTF` API in Android 5.x
  if (api_level < __ANDROID_API_M__) {  // Build.VERSION_CODES.M
    base::android::ScopedLocalJavaRef<jbyteArray> jni_key =
        base::android::JNIConvertHelper::ConvertToJNIByteArray(env, key);
    base::android::ScopedLocalJavaRef<jbyteArray> jni_value =
        base::android::JNIConvertHelper::ConvertToJNIByteArray(env, value);
    Java_JavaOnlyMap_putByteArrayAsString(env, jni_object_.Get(), jni_key.Get(),
                                          jni_value.Get());
  } else {
    base::android::ScopedLocalJavaRef<jstring> jni_key =
        base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, key);
    base::android::ScopedLocalJavaRef<jstring> jni_value =
        base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, value);
    Java_JavaOnlyMap_putString(env, jni_object_.Get(), jni_key.Get(),
                               jni_value.Get());
  }
}

void JavaOnlyMap::PushInt(const std::string& key, int value) {
  PushInt(key.c_str(), value);
}

void JavaOnlyMap::PushInt(const char* key, int value) {
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedLocalJavaRef<jstring> jni_key =
      base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, key);
  Java_JavaOnlyMap_putInt(env, jni_object_.Get(), jni_key.Get(), value);
}

void JavaOnlyMap::PushDouble(const std::string& key, double value) {
  PushDouble(key.c_str(), value);
}

void JavaOnlyMap::PushDouble(const char* key, double value) {
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedLocalJavaRef<jstring> jni_key =
      base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, key);
  Java_JavaOnlyMap_putDouble(env, jni_object_.Get(), jni_key.Get(), value);
}

void JavaOnlyMap::PushArray(const std::string& key, JavaOnlyArray* value) {
  PushArray(key.c_str(), value);
}

void JavaOnlyMap::PushArray(const char* key, JavaOnlyArray* value) {
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedLocalJavaRef<jstring> jni_key =
      base::android::JNIConvertHelper::ConvertToJNIStringUTF(env, key);
  Java_JavaOnlyMap_putArray(env, jni_object_.Get(), jni_key.Get(),
                            value->jni_object());
}

lynx::base::android::ScopedLocalJavaRef<jobject>
JavaOnlyMap::JavaOnlyMapGetKeys(JNIEnv* env, jobject map) {
  return Java_JavaOnlyMap_getKeys(env, map);
}

ReadableType JavaOnlyMap::JavaOnlyMapGetTypeAtIndex(JNIEnv* env, jobject map,
                                                    jstring key) {
  return static_cast<ReadableType>(
      Java_JavaOnlyMap_getTypeIndex(env, map, key));
}

std::string JavaOnlyMap::JavaOnlyMapGetStringAtIndex(JNIEnv* env, jobject map,
                                                     jstring key) {
  return base::android::JNIConvertHelper::ConvertToString(
      env, Java_JavaOnlyMap_getString(env, map, key).Get());
}

}  // namespace android
}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForJavaOnlyMap(JNIEnv* env) { return RegisterNativesImpl(env); }
}  // namespace jni
}  // namespace animax
