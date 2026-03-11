// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "base/include/platform/android/jni_convert_helper.h"
#include "platform/android/animax_android/src/main/jni/gen/NativePlayerCallback_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/NativePlayerCallback_register_jni.h"
#include "src/video/android/video_player_android.h"

static void OnError(JNIEnv* env, jclass jcaller, jlong nativePlayer,
                    jstring errMsg) {
  if (!nativePlayer) {
    return;
  }
  auto* player =
      reinterpret_cast<lynx::animax::VideoPlayerAndroid*>(nativePlayer);
  player->NotifyErrorEvent(
      lynx::base::android::JNIConvertHelper::ConvertToString(env, errMsg));
}

namespace lynx {
namespace animax {}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForNativePlayerCallback(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
