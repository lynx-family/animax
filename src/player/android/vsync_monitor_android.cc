// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/vsync_monitor_android.h"

#include "base/include/platform/android/jni_utils.h"
#include "platform/android/animax_android/src/main/jni/gen/VSyncMonitor_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/VSyncMonitor_register_jni.h"
#include "src/base/thread/thread_assert.h"

static void InvokeCallback(JNIEnv* env, jclass jcaller,
                           jlong native_callback_ptr, jlong frame_time_nanos) {
  auto* callback = reinterpret_cast<lynx::animax::VSyncMonitor::Callback*>(
      native_callback_ptr);
  (*callback)(frame_time_nanos);
  delete callback;
}

namespace lynx {
namespace animax {

void VSyncMonitorAndroid::RequestVSync(VSyncMonitor::Callback callback) {
  auto* callback_ptr = new Callback{std::move(callback)};
  JNIEnv* env = base::android::AttachCurrentThread();
  // The callback_ptr is first passed to VSyncMonitor.requestVSync, and then put
  // within a Choreographer.FrameCallback instance. Once the
  // Choreographer.FrameCallback instance is invoked upon VSync, the pointer is
  // passed to "InvokeCallback". The pointer will be invoked and then deleted
  // within "InvokeCallback", ensuring proper lifecycle management.
  Java_VSyncMonitor_requestVSync(env, reinterpret_cast<jlong>(callback_ptr));
}

}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForVSyncMonitor(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
