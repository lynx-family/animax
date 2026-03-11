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

AudioController::AudioController(std::unique_ptr<AudioPlayer> player)
    : audio_player_(std::move(player)) {
  DCHECK(audio_player_);
}

void AudioController::Init(const std::string &file_path) {
  audio_player_->Init(file_path);
}

void AudioController::OnProgress(double progress) {
  if (can_play_) {
    if (!is_playing_) {
      audio_player_->SeekToProgress(progress);
      audio_player_->Resume();
      is_playing_ = true;
    } else if (audio_player_->NeedSync(progress)) {
      audio_player_->SeekToProgress(progress);
    }
  }
}

void AudioController::OnResume() { can_play_ = true; }

void AudioController::OnPause() {
  is_playing_ = false;
  can_play_ = false;
  audio_player_->Pause();
}

void AudioController::SetVolume(double volume) {
  audio_player_->SetVolume(volume);
}

}  // namespace animax
}  // namespace lynx
