// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_PROPERTY_CALLBACK_ANDROID_H_
#define ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_PROPERTY_CALLBACK_ANDROID_H_

#include "base/include/platform/android/jni_utils.h"
#include "include/property/property_update_response.h"

namespace lynx {
namespace animax {

class AnimaXPropertyCallbackAndroid {
 public:
  AnimaXPropertyCallbackAndroid(JNIEnv* env, jobject callback)
      : callback_(env, callback) {}

  void OnPropertyCallback(const PropertyUpdateResponse& response);

 private:
  base::android::ScopedGlobalJavaRef<jobject> callback_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_PROPERTY_CALLBACK_ANDROID_H_
