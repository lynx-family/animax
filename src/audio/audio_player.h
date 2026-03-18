// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_AUDIO_AUDIO_PLAYER_H_
#define ANIMAX_SRC_AUDIO_AUDIO_PLAYER_H_

#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>
#include <queue>

#include "src/base/log/log.h"

namespace lynx {
namespace animax {
class AudioAsset;
class AnimaXAbility;
class AudioPlayer {
 public:
  static constexpr double kBaseSyncThreshold = 50.f;
  static constexpr double kRecordingScale = 200.f;
  static constexpr int kMaxNeedSyncTimeInRecord = 3;
  static constexpr double kIncreaseFactorStep = 2.f;
  static constexpr int kMaxIncreaseFactorTime = 3;

  virtual ~AudioPlayer() = default;
  virtual void Resume() = 0;
  virtual void Pause() = 0;
  virtual void SeekToProgress(double progress) = 0;

  bool NeedSync(double progress) {
    auto duration = GetDuration();
    auto animation_time = progress * duration;
    auto diff = std::abs(GetAudioTime() - animation_time);
    auto diff_in_loop = std::abs(duration - diff);
    auto threshold = kBaseSyncThreshold * factor_;
    bool ret = diff > threshold && diff_in_loop > threshold;
    if (ret && increase_factor_time_ <= kMaxIncreaseFactorTime) {
      auto now = std::chrono::steady_clock::now();
      int64_t current_time_ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              now.time_since_epoch())
              .count();

      // Pop out expired timestamps outside the recording scale
      while (!seek_timestamps_.empty() &&
             (current_time_ms - seek_timestamps_.front() > kRecordingScale)) {
        seek_timestamps_.pop();
      }

      if (seek_timestamps_.size() + 1 >= kMaxNeedSyncTimeInRecord) {
        factor_ += kIncreaseFactorStep;
        increase_factor_time_++;
        ANIMAX_LOGW("AudioPlayer: Frequent seek detected in "
                    << increase_factor_time_ << " times, new threshold: "
                    << (kBaseSyncThreshold * factor_) << "ms")
        while (!seek_timestamps_.empty()) {
          seek_timestamps_.pop();
        }
      } else {
        seek_timestamps_.push(current_time_ms);
      }
    }
    return ret;
  }

  static std::unique_ptr<AudioPlayer> MakeAudioPlayer(
      std::shared_ptr<AnimaXAbility> ability,
      std::shared_ptr<AudioAsset> asset);

 protected:
  AudioPlayer() {}
  virtual double GetDuration() = 0;
  virtual double GetAudioTime() = 0;

 private:
  std::queue<int64_t> seek_timestamps_;
  double factor_ = 1.0;
  int increase_factor_time_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_AUDIO_AUDIO_PLAYER_H_
