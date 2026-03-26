// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// Copyright 2018 Airbnb, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/layer/layer_factory.h"

#include "src/layer/alpha_video_layer.h"
#include "src/layer/audio_layer.h"
#include "src/layer/camera_layer.h"
#include "src/layer/composition_layer.h"
#include "src/layer/image_layer.h"
#include "src/layer/null_layer.h"
#include "src/layer/shape_layer.h"
#include "src/layer/solid_layer.h"
#include "src/layer/text_layer.h"

namespace lynx {
namespace animax {

std::unique_ptr<BaseLayer> LayerFactory::CreateLayer(
    LayerModel& layer_model, CompositionModel& composition) {
  auto layer_type = layer_model.GetLayerType();
  std::unique_ptr<BaseLayer> layer;
  switch (layer_type) {
    case LayerType::kShape:
      layer =
          std::unique_ptr<ShapeLayer>(new ShapeLayer(layer_model, composition));
      break;
    case LayerType::kPreComp: {
      auto pre_composition_layer = std::unique_ptr<CompositionLayer>(
          new CompositionLayer(layer_model, composition));
      auto& precomps = composition.GetPrecomps();
      if (precomps.find(layer_model.GetRefId()) != precomps.end()) {
        pre_composition_layer->SetLayerModels(precomps[layer_model.GetRefId()]);
      }
      layer = std::move(pre_composition_layer);
      break;
    }
    case LayerType::kSolid:
      layer =
          std::unique_ptr<SolidLayer>(new SolidLayer(layer_model, composition));
      break;
    case LayerType::kImage:
      layer =
          std::unique_ptr<ImageLayer>(new ImageLayer(layer_model, composition));
      break;
    case LayerType::kNull:
      layer =
          std::unique_ptr<NullLayer>(new NullLayer(layer_model, composition));
      break;
    case LayerType::kText:
      layer =
          std::unique_ptr<TextLayer>(new TextLayer(layer_model, composition));
      break;
    case LayerType::kCamera:
      layer = std::unique_ptr<CameraLayer>(
          new CameraLayer(layer_model, composition));
      break;
    case LayerType::kAlphaVideo:
      layer = std::unique_ptr<AlphaVideoLayer>(
          new AlphaVideoLayer(layer_model, composition));
      break;
    case LayerType::kAudio:
      layer =
          std::unique_ptr<AudioLayer>(new AudioLayer(layer_model, composition));
      break;
    default:
      ANIMAX_LOGI("skip layer type:"
                  << std::to_string(static_cast<int32_t>(layer_type)));
  }
  return layer;
}

}  // namespace animax
}  // namespace lynx
