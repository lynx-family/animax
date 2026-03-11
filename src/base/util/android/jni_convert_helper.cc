// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/android/jni_convert_helper.h"

#include "base/include/platform/android/java_type.h"

namespace lynx {
namespace animax {
namespace android {

lynx::base::android::ScopedLocalJavaRef<jstring>
JNIConvertHelper::ConvertToJNIStringUTF(JNIEnv* env, const std::string& value) {
  jstring str = env->NewStringUTF(value.c_str());  // NOLINT
  return lynx::base::android::ScopedLocalJavaRef<jstring>(env, str);
}

lynx::base::android::ScopedLocalJavaRef<jstring>
JNIConvertHelper::ConvertToJNIStringUTF(JNIEnv* env, const char* value) {
  jstring str = env->NewStringUTF(value);  // NOLINT
  return lynx::base::android::ScopedLocalJavaRef<jstring>(env, str);
}

std::vector<uint8_t> JNIConvertHelper::ConvertJavaBinary(JNIEnv* env,
                                                         jobject j_object) {
  if (env == nullptr || j_object == nullptr) {
    return {};
  }

  lynx::base::android::ScopedLocalJavaRef<jclass> byte_buffer_class(
      env, env->FindClass("java/nio/ByteBuffer"));
  LYNX_BASE_DCHECK(!byte_buffer_class.IsNull());

  jmethodID array_method =
      env->GetMethodID(byte_buffer_class.Get(), "array", "()[B");
  jmethodID position_method =
      env->GetMethodID(byte_buffer_class.Get(), "position", "()I");
  jmethodID limit_method =
      env->GetMethodID(byte_buffer_class.Get(), "limit", "()I");
  if (array_method == nullptr || position_method == nullptr ||
      limit_method == nullptr) {
    return {};
  }

  lynx::base::android::ScopedLocalJavaRef<jbyteArray> bytes(
      env,
      static_cast<jbyteArray>(env->CallObjectMethod(j_object, array_method)));
  jint position = env->CallIntMethod(j_object, position_method);
  jint limit = env->CallIntMethod(j_object, limit_method);
  if (env->ExceptionCheck()) {
    env->ExceptionClear();
    return {};
  }

  std::vector<uint8_t> binary;
  if (!bytes.IsNull()) {
    jbyte* temp = env->GetByteArrayElements(bytes.Get(), JNI_FALSE);
    if (position >= 0 && limit >= position) {
      binary.assign(reinterpret_cast<const uint8_t*>(temp) + position,
                    reinterpret_cast<const uint8_t*>(temp) + limit);
    }
    env->ReleaseByteArrayElements(bytes.Get(), temp, JNI_FALSE);
  }
  return binary;
}

}  // namespace android
}  // namespace animax
}  // namespace lynx
