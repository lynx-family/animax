// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ANDROID_ANIMAX_COMPOSITION_CALLBACK_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ANDROID_ANIMAX_COMPOSITION_CALLBACK_ANDROID_H_

#include "base/include/platform/android/jni_utils.h"
#include "src/model/android/animax_composition_android.h"

namespace lynx {
namespace animax {

class AnimaXCompositionCallbackAndroid {
 public:
  AnimaXCompositionCallbackAndroid(JNIEnv* env, jobject callback)
      : callback_(env, callback) {}

  void OnCompositionCallback(jobject composition, const std::string& error);

 private:
  base::android::ScopedGlobalJavaRef<jobject> callback_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ANDROID_ANIMAX_COMPOSITION_CALLBACK_ANDROID_H_
