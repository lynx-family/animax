// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_ANDROID_JAVA_ONLY_MAP_H_
#define ANIMAX_SRC_BASE_UTIL_ANDROID_JAVA_ONLY_MAP_H_

#include <memory>
#include <string>

#include "base/include/platform/android/scoped_java_ref.h"
#include "src/base/util/android/java_only_array.h"

namespace lynx {
namespace animax {
namespace android {
class JavaOnlyArray;
class JavaOnlyMap {
 public:
  JavaOnlyMap();

  JavaOnlyMap(JNIEnv* env,
              lynx::base::android::ScopedLocalJavaRef<jobject>& ref)
      : jni_object_(env, ref.Get()) {}

  ~JavaOnlyMap() = default;

  void PushString(const std::string& key, const std::string& value);
  void PushString(const char* key, const char* value);
  void PushInt(const std::string& key, int value);
  void PushInt(const char* key, int value);
  void PushDouble(const std::string& key, double value);
  void PushDouble(const char* key, double value);
  void PushArray(const std::string& key, JavaOnlyArray* value);
  void PushArray(const char* key, JavaOnlyArray* value);

  inline jobject jni_object() { return jni_object_.Get(); }

  static lynx::base::android::ScopedLocalJavaRef<jobject> JavaOnlyMapGetKeys(
      JNIEnv* env, jobject map);
  static ReadableType JavaOnlyMapGetTypeAtIndex(JNIEnv* env, jobject map,
                                                jstring key);
  static std::string JavaOnlyMapGetStringAtIndex(JNIEnv* env, jobject map,
                                                 jstring key);

 private:
  base::android::ScopedGlobalJavaRef<jobject> jni_object_;
};

}  // namespace android
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_ANDROID_JAVA_ONLY_MAP_H_
