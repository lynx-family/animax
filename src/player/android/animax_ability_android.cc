// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/animax_ability_android.h"

#include "platform/android/animax_android/src/main/jni/gen/BaseAbility_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/BaseAbility_register_jni.h"

namespace animax {
namespace jni {
bool RegisterJNIForBaseAbility(JNIEnv* env) { return RegisterNativesImpl(env); }
}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

AnimaXAbilityAndroid::AnimaXAbilityAndroid(JNIEnv* env, jobject ability)
    : ability_(env, ability) {}

void AnimaXAbilityAndroid::SendEvent(uint8_t event, jobject map) {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  Java_BaseAbility_sendEvent(env, ability_.Get(), event, map);
}

bool AnimaXAbilityAndroid::DisableByteVC1Decoder() const {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  return Java_BaseAbility_disableByteVC1Decoder(env, ability_.Get());
}

bool AnimaXAbilityAndroid::IsDownsampleVideoEnabled() const {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  return Java_BaseAbility_isDownsampleVideoEnabled(env, ability_.Get());
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXAbilityAndroid::CreateVideoPlayer(jlong native_ptr) const {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  return Java_BaseAbility_createVideoPlayer(env, ability_.Get(), native_ptr);
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXAbilityAndroid::CreateAudioPlayer(jobject asset_object) const {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  return Java_BaseAbility_createAudioPlayer(env, ability_.Get(), asset_object);
}

}  // namespace animax
}  // namespace lynx
