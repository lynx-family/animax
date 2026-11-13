// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/android/animax_text_measure_callback_android.h"

#include "platform/android/animax_android/src/main/jni/gen/AnimaXTextMeasureCallback_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXTextMeasureCallback_register_jni.h"

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXTextMeasureCallback(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

void AnimaXTextMeasureCallbackAndroid::OnComplete(bool success, float width) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_AnimaXTextMeasureCallback_onTextMeasureComplete(env, callback_.Get(),
                                                       success, width);
}

}  // namespace animax
}  // namespace lynx
