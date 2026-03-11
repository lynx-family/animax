// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/android/metrics_callback_android.h"

#include "platform/android/animax_android/src/main/jni/gen/AnimaXMetricsCallback_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXMetricsCallback_register_jni.h"

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXMetricsCallback(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

void MetricsCallbackAndroid::OnMetricsReady(jobject metrics) {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  Java_AnimaXMetricsCallback_onMetricsReady(env, java_callback_.Get(), metrics,
                                            java_events_map_.Get());
}

}  // namespace animax
}  // namespace lynx
