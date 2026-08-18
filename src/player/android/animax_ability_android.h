// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_ABILITY_ANDROID_H_
#define ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_ABILITY_ANDROID_H_

#include "base/include/closure.h"
#include "base/include/platform/android/jni_convert_helper.h"
#include "include/base/macros.h"
#include "src/player/animax_ability.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT AnimaXAbilityAndroid : public AnimaXAbility {
 public:
  explicit AnimaXAbilityAndroid(JNIEnv* env, jobject ability);

  void SendEvent(uint8_t event, jobject map);

  bool DisableByteVC1Decoder() const;

  bool IsDownsampleVideoEnabled() const override;

  base::android::ScopedLocalJavaRef<jobject> CreateVideoPlayer(
      jlong native_ptr) const;

  base::android::ScopedLocalJavaRef<jobject> CreateAudioPlayer(
      jobject asset_object) const;

 private:
  base::android::ScopedGlobalJavaRef<jobject> ability_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_ABILITY_ANDROID_H_
