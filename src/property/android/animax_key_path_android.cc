// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/android/animax_key_path_android.h"

#include "platform/android/animax_android/src/main/jni/gen/AnimaXKeyPath_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXKeyPath_register_jni.h"
#include "src/base/util/android/java_only_array.h"

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXKeyPath(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

AnimaXKeyPathAndroid::AnimaXKeyPathAndroid(JNIEnv* env, jobject key_path)
    : AnimaXKeyPath([&] {
        auto keys = Java_AnimaXKeyPath_getKeys(env, key_path);
        std::vector<std::string> key_vector;
        for (int i = 0;
             i < android::JavaOnlyArray::JavaOnlyArrayGetSize(env, keys.Get());
             i++) {
          if (android::JavaOnlyArray::JavaOnlyArrayGetTypeAtIndex(
                  env, keys.Get(), i) == android::ReadableType::String) {
            key_vector.push_back(
                android::JavaOnlyArray::JavaOnlyArrayGetStringAtIndex(
                    env, keys.Get(), i));
          }
        }
        return key_vector;
      }()) {}

}  // namespace animax
}  // namespace lynx
