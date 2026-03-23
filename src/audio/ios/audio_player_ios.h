// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_AUDIO_IOS_AUDIO_PLAYER_IOS_H_
#define ANIMAX_SRC_AUDIO_IOS_AUDIO_PLAYER_IOS_H_

#include <AudioToolbox/AudioToolbox.h>

#include <memory>
#include <mutex>

#include "src/audio/audio_player.h"
#include "src/audio/ios/audio_asset_ios.h"

namespace lynx {
namespace animax {

class AudioPlayerIOS : public AudioPlayer {
 public:
  AudioPlayerIOS() = default;
  ~AudioPlayerIOS() override;

  bool Init(std::shared_ptr<AudioAsset> asset);

  void Resume() override;
  void Pause() override;
  void SeekToProgress(double progress) override;

 private:
  double GetDuration() override;
  double GetAudioTime() override;
  // Apple's Core Audio documentation generally advises against using
  // std::mutex, Objective-C @synchronized, or blocking calls inside
  // AURenderCallback to prevent Priority Inversion and UI thread blocking which
  // can cause audio dropouts (glitches).
  //
  // However, in this specific architecture:
  // 1. All audio data is already fully decoded in-memory via AudioAssetIOS.
  // 2. The critical section only involves updating a few integers and a very
  //    small memcpy (e.g., a few hundred frames), which takes mere nanoseconds.
  // 3. GPU thread operations (SeekToProgress, GetAudioTime) acquire the lock
  //    extremely briefly and will never block the render thread meaningfully.
  //
  // Attempting a strictly lock-free implementation here would require a complex
  // SeqLock to prevent "Snapshot Tearing" when reading multiple dependent time
  // anchors (anchor_read_frame_, anchor_hardware_sample_time_, etc.). Using
  // std::atomic<Struct> for a 24-byte payload degrades to an implicit global
  // lock under the hood anyway. Therefore, an explicit std::mutex is chosen as
  // the most robust, simple, and performant solution to guarantee cross-thread
  // state consistency.
  static OSStatus RenderCallback(void* inRefCon,
                                 AudioUnitRenderActionFlags* ioActionFlags,
                                 const AudioTimeStamp* inTimeStamp,
                                 UInt32 inBusNumber, UInt32 inNumberFrames,
                                 AudioBufferList* ioData);

  std::shared_ptr<AudioAssetIOS> asset_ios_;
  AudioUnit audio_unit_ = nullptr;

  // Pre-calculated invariant values to avoid computation in the hot path.
  // Read-only after Init().
  int32_t bytes_per_frame_ = 0;
  int64_t total_frames_ = 0;
  double sample_rate_ = 0.0;
  double duration_ms_ = 0.0;

  // Mutex protects all time anchors and read cursors below.
  // Ensures UI thread and Audio Render thread share a perfectly consistent
  // snapshot.
  std::mutex state_mutex_;

  // Current read cursor (in frames) pointing to the in-memory asset.
  int64_t current_read_frame_ = 0;

  // Time anchors for accurate A/V sync based on mSampleTime.
  int64_t anchor_read_frame_ = 0;
  Float64 anchor_hardware_sample_time_ = 0.0;
  Float64 current_hardware_sample_time_ = 0.0;
};

}  // namespace animax
}  // namespace lynx
#endif  // ANIMAX_SRC_AUDIO_IOS_AUDIO_PLAYER_IOS_H_
