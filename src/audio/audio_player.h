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
class AudioPlayer {
 public:
  // Make this class pure virtual in next commit.
  virtual ~AudioPlayer() = default;
  virtual void Init(const std::string& file_path) {}
  virtual void Resume() {}
  virtual void Pause() {}
  // todo(lixianruo.cyrus): support speed change.

  // Android don't support it until API 23 (android 6)
  // virtual void SetSpeed(double speed){}
  virtual void SetVolume(double volume) {}
  virtual void SeekToProgress(double progress) {}
  bool NeedSync(double progress) {
    auto layer_current_time = progress * GetDuration();
    return std::abs(GetCurrentTime() - layer_current_time) > 1;
  }

 protected:
  virtual double GetDuration() { return 0; }
  virtual double GetCurrentTime() { return 0; }
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_AUDIO_AUDIO_PLAYER_H_
