// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "base/include/no_destructor.h"
#include "base/include/platform/android/jni_utils.h"
#include "platform/android/animax_android/src/main/jni/gen/VSyncMonitor_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/VSyncMonitor_register_jni.h"
#include "src/player/vsync_dispatcher.h"

static void InvokeCallback(JNIEnv* env, jclass jcaller,
                           jlong frame_time_nanos) {
  lynx::animax::VSyncDispatcher::Instance().OnVSync(frame_time_nanos);
}

namespace lynx {
namespace animax {
class VsyncDispatcherAndroid final : public VSyncDispatcher {
 public:
  void RequestVSync() override {
    JNIEnv* env = base::android::AttachCurrentThread();
    Java_VSyncMonitor_requestVSync(env);
  }
};

VSyncDispatcher& VSyncDispatcher::Instance() {
  static base::NoDestructor<VsyncDispatcherAndroid> inst;
  return *inst;
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
