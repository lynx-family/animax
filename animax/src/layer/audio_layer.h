// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_AUDIO_LAYER_H_
#define ANIMAX_SRC_LAYER_AUDIO_LAYER_H_

#include <memory>

#include "src/layer/base_layer.h"
#include "src/resource/asset/audio_asset.h"

namespace lynx {
namespace animax {

class AudioLayer : public BaseLayer {
 public:
  AudioLayer(LayerModel& layer, CompositionModel& composition);

  void Init() override;

  // We need to trigger audio controller in this function so that we can reuse
  // the visible/hidden logic in base layer.
  void DrawLayer(Canvas& canvas, Matrix& matrix, int32_t alpha) override;

 private:
  double GetCurrentProgress();
  void AttachToAssetOnce();
  std::shared_ptr<AudioAsset> audio_asset_;
  std::shared_ptr<AudioController> audio_controller_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_AUDIO_LAYER_H_
