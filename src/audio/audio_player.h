// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_AUDIO_AUDIO_PLAYER_H_
#define ANIMAX_SRC_AUDIO_AUDIO_PLAYER_H_

#include <atomic>
#include <memory>

namespace lynx {
namespace animax {
class AudioAsset;
class AnimaXAbility;
class AudioPlayer {
 public:
  static constexpr double kSyncThreshold = 50.f;
  virtual ~AudioPlayer() = default;
  virtual void Resume() = 0;
  virtual void Pause() = 0;
  virtual void SeekToProgress(double progress) = 0;

  bool NeedSync(double progress) {
    auto duration = GetDuration();
    auto animation_time = progress * duration;
    auto diff = std::abs(GetAudioTime() - animation_time);
    auto diff_in_loop = std::abs(duration - diff);
    return diff > kSyncThreshold && diff_in_loop > kSyncThreshold;
  }

  static std::unique_ptr<AudioPlayer> MakeAudioPlayer(
      std::shared_ptr<AnimaXAbility> ability,
      std::shared_ptr<AudioAsset> asset);

 protected:
  AudioPlayer() {}
  virtual double GetDuration() = 0;
  virtual double GetAudioTime() = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_AUDIO_AUDIO_PLAYER_H_
