// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
//
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

#ifndef ANIMAX_SRC_MODEL_COMPOSITION_MODEL_H_
#define ANIMAX_SRC_MODEL_COMPOSITION_MODEL_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "src/layer/font_asset_manager.h"
#include "src/model/marker_model.h"
#include "src/model/rect_model.h"
#include "src/resource/asset/audio_asset.h"
#include "src/resource/asset/font_asset.h"
#include "src/resource/asset/image_asset.h"
#include "src/resource/asset/video_asset.h"

namespace lynx {
namespace animax {

class LayerModel;
class FontCharacterModel;

using LayerModelList = std::vector<std::unique_ptr<LayerModel>>;

struct ParseContext {
  bool enable_audio_;
  // Only updated on GradientColor parse
  int32_t color_points_;
};

class CompositionModel {
 public:
  CompositionModel(float scale);
  ~CompositionModel();

  void Init(std::unique_ptr<RectF> bounds, float start_frame, float end_frame,
            float frame_rate, bool enable_3d);

  long GetDuration();
  float GetStartFrame() { return start_frame_; }
  float GetEndFrame() { return end_frame_; }
  float GetScale() { return scale_; }
  float GetDurationFrames() { return end_frame_ - start_frame_; }
  float GetFrameRate() { return frame_rate_; }

  RectF& GetBounds() { return *bounds_; }

  LayerModelList& GetLayers() { return layers_; }
  std::unordered_map<int32_t, LayerModel*>& GetLayerMap() { return layer_map_; }

  std::unordered_map<std::string, LayerModelList>& GetPrecomps() {
    return pre_comps_;
  }
  std::unordered_map<std::string, std::shared_ptr<ImageAsset>>& GetImages() {
    return images_;
  }
  std::unordered_map<std::string, std::shared_ptr<VideoAsset>>& GetVideos() {
    return videos_;
  }
  std::unordered_map<std::string, std::shared_ptr<AudioAsset>>& GetAudios() {
    return audios_;
  }

  // key: font name, value: font asset
  std::unordered_map<std::string, std::shared_ptr<FontAsset>>& GetFonts() {
    return font_asset_manager_->GetFontAssetMap();
  }
  std::unordered_map<int32_t, std::unique_ptr<FontCharacterModel>>&
  GetCharacters() {
    return characters_;
  }

  FontAssetManager& GetFontAssetManager() { return *font_asset_manager_; }

  std::vector<std::unique_ptr<MarkerModel>>& GetMarkers() { return markers_; }
  MarkerModel* GetMarker(const std::string& marker_name);

  void SetHashDashPattern(bool has_dash_patern);
  void IncrementMatteOrMaskCount(int32_t count);

  Asset* GetResource(ResourceType type, const std::string& id);

  ParseContext parse_context_;

  // This flag controls whether the AnimatableTransformModel should retain
  // default property values (identity).
  bool should_reset_identity_ = true;

  void SetAssetsLoaded(bool is_loaded) {
    is_assets_loaded_.store(is_loaded, std::memory_order_release);
  }

  bool IsAssetsLoaded() const {
    return is_assets_loaded_.load(std::memory_order_acquire);
  }

  bool GetEnable3D() const { return enable_3d_; }

 private:
  std::unique_ptr<RectF> bounds_;

  int32_t mask_and_matte_count_ = 0;
  float start_frame_ = 0;
  float end_frame_ = 0;
  float frame_rate_ = 30;

  float scale_ = 1;
  bool has_dash_pattern_ = false;
  bool enable_3d_ = false;

  // parse assets
  std::unordered_map<std::string, LayerModelList> pre_comps_;
  std::unordered_map<std::string, std::shared_ptr<ImageAsset>> images_;
  std::unordered_map<std::string, std::shared_ptr<VideoAsset>> videos_;
  std::unordered_map<std::string, std::shared_ptr<AudioAsset>> audios_;

  std::atomic<bool> is_assets_loaded_{false};

  // parse layers
  LayerModelList layers_;
  // hold pointers only
  std::unordered_map<int32_t, LayerModel*> layer_map_;

  // text
  std::unordered_map<int32_t, std::unique_ptr<FontCharacterModel>> characters_;
  std::unique_ptr<FontAssetManager> font_asset_manager_;

  // marker
  std::vector<std::unique_ptr<MarkerModel>> markers_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_COMPOSITION_MODEL_H_
