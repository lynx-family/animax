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
    audio_asset_ = audio.get();
  } else {
    ANIMAX_LOGE("audio " << id << " is null");
  }
}

void AudioLayer::Init() {
  BaseLayer::Init();
  if (!audio_asset_) {
    return;
  }
  audio_controller_ =
      std::make_shared<AudioController>(std::make_unique<AudioPlayer>());
  audio_controller_->Init(audio_asset_->GetLocalPath());
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

void AudioLayer::DrawLayer(Canvas &canvas, Matrix &matrix, int32_t alpha) {
  if (!audio_controller_) {
    return;
  }
  audio_controller_->OnProgress(GetCurrentProgress());
}

double AudioLayer::GetCurrentProgress() {
  auto &keyframe = in_out_animation_->GetCurrentKeyframe();
  float progress =
      in_out_animation_->GetProgress() - keyframe.GetStartProgress();
  if (progress < 0) {
    return 0;
  }
  if (progress > 1) {
    return 1;
  }
  return progress;
}

}  // namespace animax
}  // namespace lynx
