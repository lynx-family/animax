// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/audio/audio_controller.h"

#include "src/audio/audio_player.h"
#include "src/base/log/log.h"
#include "src/player/animax_main_controller.h"
#include "src/player/animax_renderer.h"

namespace lynx {
namespace animax {

void AudioController::OnProgress(double progress) {
  if (!audio_player_ || !can_play_ || mute_) {
    return;
  }

  // Compares the timestamp of animation and audio.
  if (audio_player_->NeedSync(progress)) {
    // Flush the audio buffer and update the head to current frame.
    audio_player_->SeekToProgress(progress);
  }

  // AudioTrack don't have a data request callback until Android 10,
  // feed it every frame to prevent audio render from starving.
  // No side effects on other platforms.
  audio_player_->Resume();
  is_playing_ = true;
}

void AudioController::OnResume() { can_play_ = true; }

void AudioController::OnPause() {
  is_playing_ = false;
  can_play_ = false;
  if (audio_player_) {
    audio_player_->Pause();
  }
}

void AudioController::SetMuted(bool mute) {
  mute_ = mute;
  if (mute_ && is_playing_) {
    is_playing_ = false;
    if (audio_player_) {
      audio_player_->Pause();
    }
  }
}

void AudioController::SetAudioPlayer(std::unique_ptr<AudioPlayer> player) {
  audio_player_ = std::move(player);
}

bool AudioController::HasAudioPlayer() { return audio_player_ != nullptr; }

}  // namespace animax
}  // namespace lynx
