// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/animax_context_android.h"

#include "platform/android/animax_android/src/main/jni/gen/AnimaXContext_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXContext_register_jni.h"

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXContext(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

AnimaXContextAndroid::AnimaXContextAndroid(JNIEnv* env, jobject java_context)
    : java_context_(
          base::android::ScopedGlobalJavaRef<jobject>(env, java_context)) {}

AnimaXContextAndroid::~AnimaXContextAndroid() = default;

base::android::ScopedLocalJavaRef<jobject> AnimaXContextAndroid::GetAbility()
    const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXContext_getAbility(env, java_context_.Get());
}

bool AnimaXContextAndroid::MultiThreadAccelerate() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXContext_multiThreadAccelerate(env, java_context_.Get());
}

bool AnimaXContextAndroid::DisablePlaybackOnAssetLoadFailure() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXContext_disablePlaybackOnAssetLoadFailure(
      env, java_context_.Get());
}

float AnimaXContextAndroid::GetDeviceDensity() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXContext_getDeviceDensity(env, java_context_.Get());
}

}  // namespace animax
}  // namespace lynx
