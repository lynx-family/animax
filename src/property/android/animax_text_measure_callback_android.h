// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_TEXT_MEASURE_CALLBACK_ANDROID_H_
#define ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_TEXT_MEASURE_CALLBACK_ANDROID_H_

#include "base/include/platform/android/jni_utils.h"

namespace lynx {
namespace animax {

class AnimaXTextMeasureCallbackAndroid {
 public:
  AnimaXTextMeasureCallbackAndroid(JNIEnv* env, jobject callback)
      : callback_(env, callback) {}

  void OnComplete(bool success, float width);

 private:
  base::android::ScopedGlobalJavaRef<jobject> callback_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_ANDROID_ANIMAX_TEXT_MEASURE_CALLBACK_ANDROID_H_
