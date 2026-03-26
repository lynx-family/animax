// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/android/audio_asset_android.h"

#include "platform/android/animax_android/src/main/jni/gen/AudioAsset_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AudioAsset_register_jni.h"
#include "src/base/log/log.h"
#include "src/base/util/android/jni_convert_helper.h"

namespace animax {
namespace jni {
bool RegisterJNIForAudioAsset(JNIEnv* env) { return RegisterNativesImpl(env); }
}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

std::shared_ptr<AudioAsset> AudioAsset::Make(AudioAssetModel model) {
  return std::make_shared<AudioAssetAndroid>(model);
}

AudioAssetAndroid::AudioAssetAndroid(AudioAssetModel model)
    : AudioAsset(std::move(model)) {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  asset_.Reset(env, Java_AudioAsset_create(env));
  DCHECK(JavaObject());
}

void AudioAssetAndroid::LoadLocal(const std::string& file_path) {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  is_valid_ = Java_AudioAsset_loadLocal(
      env, asset_.Get(),
      android::JNIConvertHelper::ConvertToJNIStringUTF(env, file_path).Get());
}
}  // namespace animax
}  // namespace lynx
