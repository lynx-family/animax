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

  // time_ms is elapsed layer-local time, not composition-normalized progress.
  // Returns false when the timestamp or audio duration is invalid.
  bool SyncToTime(double time_ms, bool force_seek = false) {
    const double duration_ms = GetDuration();
    if (!std::isfinite(time_ms) || time_ms < 0 || !std::isfinite(duration_ms) ||
        duration_ms <= 0) {
      return false;
    }
    // Preserve source looping and use the same target for comparison and seek.
    const double target_ms = std::fmod(time_ms, duration_ms);
    // Lifecycle positioning must not count as repeated drift corrections.
    if (force_seek || NeedSync(target_ms, duration_ms)) {
      SeekToProgress(target_ms / duration_ms);
    }
    return true;
  }

  static std::unique_ptr<AudioPlayer> MakeAudioPlayer(
      std::shared_ptr<AnimaXAbility> ability,
      std::shared_ptr<AudioAsset> asset);

 protected:
  AudioPlayer() {}
  virtual double GetDuration() = 0;
  virtual double GetAudioTime() = 0;

 private:
  bool NeedSync(double target_ms, double duration_ms) {
    auto diff = std::abs(GetAudioTime() - target_ms);
    auto diff_in_loop = std::abs(duration_ms - diff);
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

  std::queue<int64_t> seek_timestamps_;
  double factor_ = 1.0;
  int increase_factor_time_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_AUDIO_AUDIO_PLAYER_H_
