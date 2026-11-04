// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/audio_layer.h"

#include "include/player/animax_player.h"
#include "src/audio/audio_player.h"
#include "src/player/animax_renderer.h"

namespace lynx {
namespace animax {

AudioLayer::AudioLayer(LayerModel &layer, CompositionModel &composition)
    : BaseLayer(layer, composition) {
  auto id = layer_model_.GetRefId();
  std::shared_ptr<AudioAsset> audio = composition.GetAudios()[id];
  if (audio) {
    audio_asset_ = audio;
  } else {
    ANIMAX_LOGE("audio " << id << " is null");
  }
}

void AudioLayer::Init() {
  BaseLayer::Init();
  audio_controller_ = std::make_shared<AudioController>();
  if (!audio_controller_) {
    return;
  }
  AttachToAssetOnce();
  auto context = weak_context_.lock();
  if (!context) {
    return;
  }
  auto renderer = context->weak_renderer_actor.lock();
  if (renderer) {
    renderer->Act([audio_controller = audio_controller_](auto &renderer) {
      renderer->AddAudioController(audio_controller);
    });
  }
}

void AudioLayer::UpdateAudio() {
  if (!visible_ || layer_model_.IsHidden() || !audio_controller_) {
    return;
  }
  AttachToAssetOnce();
  audio_controller_->OnTimeUpdate(GetCurrentTimeMs());
}

double AudioLayer::GetCurrentTimeMs() {
  auto &keyframe = in_out_animation_->GetCurrentKeyframe();
  double progress = static_cast<double>(in_out_animation_->GetProgress()) -
                    keyframe.GetStartProgress();
  if (progress < 0) {
    return 0;
  }
  // Both progress values use the composition's playback frame span. Convert
  // their difference to elapsed time before passing it to the audio player.
  return progress * composition_.GetDurationFrames() * 1000.0 /
         composition_.GetFrameRate();
}

void AudioLayer::AttachToAssetOnce() {
  if (audio_controller_->HasAudioPlayer()) {
    return;
  }
  if (!audio_asset_ || !audio_asset_->IsValid()) {
    return;
  }
  auto context = weak_context_.lock();
  if (!context) {
    return;
  }
  auto ability = context->weak_ability.lock();
  if (!ability) {
    return;
  }
  auto player = AudioPlayer::MakeAudioPlayer(ability, audio_asset_);
  if (!player) {
    return;
  }
  audio_controller_->SetAudioPlayer(std::move(player));
}

}  // namespace animax
}  // namespace lynx
