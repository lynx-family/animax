// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_AUDIO_HARMONY_AUDIO_PLAYER_HARMONY_H_
#define ANIMAX_SRC_AUDIO_HARMONY_AUDIO_PLAYER_HARMONY_H_

#include <ohaudio/native_audiorenderer.h>
#include <ohaudio/native_audiostreambuilder.h>

#include <atomic>
#include <memory>
#include <mutex>

#include "src/audio/audio_player.h"
#include "src/audio/harmony/audio_asset_harmony.h"
#include "src/base/util/harmony/scoped_object_harmony.h"

namespace lynx {
namespace animax {

class AudioPlayerHarmony : public AudioPlayer {
 public:
  AudioPlayerHarmony() = default;
  ~AudioPlayerHarmony() override;

  bool Init(std::shared_ptr<AudioAsset> asset);

  void Resume() override;
  void Pause() override;
  void SeekToProgress(double progress) override;

 protected:
  double GetDuration() override;
  double GetAudioTime() override;

 private:
  int64_t AlignByteToFrames(int64_t byte_position) const;

  static int32_t OnWriteData(OH_AudioRenderer* renderer, void* userData,
                             void* buffer, int32_t bufferLen);
  static int32_t OnStreamEvent(OH_AudioRenderer* renderer, void* userData,
                               OH_AudioStream_Event event);
  static int32_t OnInterruptEvent(OH_AudioRenderer* renderer, void* userData,
                                  OH_AudioInterrupt_ForceType type,
                                  OH_AudioInterrupt_Hint hint);
  static int32_t OnError(OH_AudioRenderer* renderer, void* userData,
                         OH_AudioStream_Result error);

  std::shared_ptr<AudioAssetHarmony> asset_harmony_;
  ScopedOHObject<OH_AudioRenderer, OH_AudioRenderer_Release> renderer_;

  int32_t bytes_per_frame_ = 0;

  std::mutex playback_mutex_;

  int64_t read_offset_ = 0;
  int64_t start_position_bytes_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_AUDIO_HARMONY_AUDIO_PLAYER_HARMONY_H_
