// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_ANDROID_JNI_CONVERT_HELPER_H_
#define ANIMAX_SRC_BASE_UTIL_ANDROID_JNI_CONVERT_HELPER_H_

#include <jni.h>

#include <vector>

#include "base/include/platform/android/scoped_java_ref.h"

namespace lynx {
namespace animax {
namespace android {

class JNIConvertHelper {
 public:
  static lynx::base::android::ScopedLocalJavaRef<jstring> ConvertToJNIStringUTF(
      JNIEnv* env, const std::string& value);

  static lynx::base::android::ScopedLocalJavaRef<jstring> ConvertToJNIStringUTF(
      JNIEnv* env, const char* value);

  static std::vector<uint8_t> ConvertJavaBinary(JNIEnv* env, jobject j_binary);
};

}  // namespace android
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_ANDROID_JNI_CONVERT_HELPER_H_
