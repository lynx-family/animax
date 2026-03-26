// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/asset/audio_asset.h"

namespace lynx {
namespace animax {

AudioAsset::AudioAsset(AudioAssetModel model) : model_{std::move(model)} {}

const AudioAssetModel &AudioAsset::Model() const { return model_; }

void AudioAsset::ResetModel(AudioAssetModel info) { model_ = std::move(info); }

bool AudioAsset::IsValid() { return is_valid_; }

}  // namespace animax
}  // namespace lynx
