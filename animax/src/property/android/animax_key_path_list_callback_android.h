// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_KEY_PATH_LIST_CALLBACK_ANDROID_H_
#define ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_KEY_PATH_LIST_CALLBACK_ANDROID_H_

#include <vector>

#include "base/include/platform/android/jni_utils.h"
#include "include/property/animax_key_path.h"

namespace lynx {
namespace animax {

class AnimaXKeyPathListCallbackAndroid {
 public:
  AnimaXKeyPathListCallbackAndroid(JNIEnv* env, jobject callback)
      : callback_(env, callback) {}

  void OnKeyPathCallback(const std::vector<AnimaXKeyPath>& key_paths);

 private:
  base::android::ScopedGlobalJavaRef<jobject> callback_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_KEY_PATH_LIST_CALLBACK_ANDROID_H_
