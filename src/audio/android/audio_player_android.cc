// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/android/audio_player_android.h"

#include "platform/android/animax_android/src/main/jni/gen/AudioPlayer_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AudioPlayer_register_jni.h"
#include "src/base/log/log.h"
#include "src/player/android/animax_ability_android.h"

namespace animax {
namespace jni {
bool RegisterJNIForAudioPlayer(JNIEnv *env) { return RegisterNativesImpl(env); }
}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

std::unique_ptr<AudioPlayer> AudioPlayer::MakeAudioPlayer(
    std::shared_ptr<AnimaXAbility> ability, std::shared_ptr<AudioAsset> asset) {
  auto asset_android = std::static_pointer_cast<AudioAssetAndroid>(asset);
  auto ability_android =
      std::static_pointer_cast<AnimaXAbilityAndroid>(ability);
  if (!asset_android || !asset_android->JavaObject() || !ability_android) {
    return nullptr;
  }
  auto player_ =
      ability_android->CreateAudioPlayer(asset_android->JavaObject());
  if (!player_.Get()) {
    return nullptr;
  }
  return std::make_unique<AudioPlayerAndroid>(std::move(player_));
}

AudioPlayerAndroid::AudioPlayerAndroid(
    base::android::ScopedLocalJavaRef<jobject> player) {
  DCHECK(player.Get());
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  player_.Reset(env, player);
}

void AudioPlayerAndroid::Resume() {
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  Java_AudioPlayer_play(env, player_.Get());
}

void AudioPlayerAndroid::Pause() {
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  Java_AudioPlayer_pause(env, player_.Get());
}

void AudioPlayerAndroid::SeekToProgress(double progress) {
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  Java_AudioPlayer_seekToProgress(env, player_.Get(), progress);
}

double AudioPlayerAndroid::GetAudioTime() {
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  return Java_AudioPlayer_getAudioTime(env, player_.Get());
}

double AudioPlayerAndroid::GetDuration() {
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  return Java_AudioPlayer_getDuration(env, player_.Get());
}

}  // namespace animax
}  // namespace lynx
