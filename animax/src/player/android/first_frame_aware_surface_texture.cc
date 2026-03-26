// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/first_frame_aware_surface_texture.h"

#include "platform/android/animax_android/src/main/jni/gen/FirstFrameAwareSurfaceTexture_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/FirstFrameAwareSurfaceTexture_register_jni.h"
#include "src/base/log/log.h"

namespace animax {
namespace jni {
bool RegisterJNIForFirstFrameAwareSurfaceTexture(JNIEnv *env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

SurfaceTextureAndroid::SurfaceTextureAndroid(JNIEnv *env,
                                             jobject surface_texture) {
  surface_texture_.Reset(env, surface_texture);
}

void SurfaceTextureAndroid::Release() { ANIMAX_LOGI("Release"); }

FirstFrameAwareSurfaceTextureAndroid::FirstFrameAwareSurfaceTextureAndroid(
    JNIEnv *env, jobject surface_texture)
    : SurfaceTextureAndroid(env, surface_texture) {}

FirstFrameAwareSurfaceTextureAndroid::~FirstFrameAwareSurfaceTextureAndroid() {
  Release();
}

void FirstFrameAwareSurfaceTextureAndroid::Release() {
  ANIMAX_LOGI("Release");
  if (surface_texture_.IsNull()) {
    ANIMAX_LOGI("Release but no ref.");
    return;
  }

  auto *env = base::android::AttachCurrentThread();
  Java_FirstFrameAwareSurfaceTexture_release(env, surface_texture_.Get());
  if (!surface_texture_.IsNull()) {
    surface_texture_.ReleaseGlobalRef(env);
  }
}
}  // namespace animax
}  // namespace lynx
