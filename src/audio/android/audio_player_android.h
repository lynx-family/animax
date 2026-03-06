// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_AUDIO_AUDIO_PLAYER_ANDROID_H_
#define ANIMAX_SRC_AUDIO_AUDIO_PLAYER_ANDROID_H_

#include "base/include/platform/android/scoped_java_ref.h"
#include "src/audio/android/audio_asset_android.h"
#include "src/audio/audio_player.h"

namespace lynx {
namespace animax {

class AudioPlayerAndroid : public AudioPlayer {
 public:
  ~AudioPlayerAndroid() override = default;
  explicit AudioPlayerAndroid(
      base::android::ScopedLocalJavaRef<jobject> player);
  void Resume() override;
  void Pause() override;
  void SeekToProgress(double progress) override;

 private:
  double GetDuration() override;
  double GetAudioTime() override;
  lynx::base::android::ScopedGlobalJavaRef<jobject> player_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_AUDIO_AUDIO_PLAYER_ANDROID_H_
