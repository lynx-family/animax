// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_ANDROID_JAVA_ONLY_ARRAY_H_
#define ANIMAX_SRC_BASE_UTIL_ANDROID_JAVA_ONLY_ARRAY_H_

#include <memory>
#include <string>

#include "base/include/platform/android/scoped_java_ref.h"

namespace lynx {
namespace animax {
namespace android {

enum ReadableType {
  Null,
  Boolean,
  Int,
  Number,
  String,
  Map,
  Array,
  Long,
  ByteArray,
  PiperData,
  LynxObject,
};

class JavaOnlyMap;
class JavaOnlyArray {
 public:
  JavaOnlyArray();
  JavaOnlyArray(JNIEnv* env,
                lynx::base::android::ScopedLocalJavaRef<jobject>& ref)
      : jni_object_(env, ref.Get()) {}

  ~JavaOnlyArray() = default;

  void PushArray(JavaOnlyArray* value);
  void PushString(const std::string& value);

  inline jobject jni_object() { return jni_object_.Get(); }

  JavaOnlyArray(JNIEnv* env, jobject jni_object)
      : jni_object_(env, jni_object) {}
  explicit JavaOnlyArray(base::android::ScopedGlobalJavaRef<jobject> ref)
      : jni_object_(ref) {}

  static int32_t JavaOnlyArrayGetSize(JNIEnv* env, jobject array);
  static ReadableType JavaOnlyArrayGetTypeAtIndex(JNIEnv* env, jobject array,
                                                  int32_t index);
  static std::string JavaOnlyArrayGetStringAtIndex(JNIEnv* env, jobject array,
                                                   int32_t index);

 private:
  base::android::ScopedGlobalJavaRef<jobject> jni_object_;
};

}  // namespace android
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_ANDROID_JAVA_ONLY_ARRAY_H_
