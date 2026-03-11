// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/android/animax_key_path_list_callback_android.h"

#include "base/include/platform/android/jni_convert_helper.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXKeyPathListCallback_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXKeyPathListCallback_register_jni.h"
#include "src/base/util/android/java_only_array.h"

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXKeyPathListCallback(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
namespace lynx {
namespace animax {

void AnimaXKeyPathListCallbackAndroid::OnKeyPathCallback(
    const std::vector<AnimaXKeyPath>& key_paths) {
  JNIEnv* env = base::android::AttachCurrentThread();

  // Create outer JavaOnlyArray to hold all key path arrays
  lynx::animax::android::JavaOnlyArray key_path_array;

  // Convert each AnimaXKeyPath to JavaOnlyArray<string> and add to outer array
  for (const auto& key_path : key_paths) {
    // Create inner JavaOnlyArray for this key path's keys
    lynx::animax::android::JavaOnlyArray keys_array;

    const auto& keys = key_path.GetKeys();
    for (const auto& key : keys) {
      keys_array.PushString(key);
    }

    // Add this key path's array to the outer array
    key_path_array.PushArray(&keys_array);
  }

  // Call Java callback method
  Java_AnimaXKeyPathListCallback_onKeyPathListCallback(
      env, callback_.Get(), key_path_array.jni_object());
}

}  // namespace animax
}  // namespace lynx
