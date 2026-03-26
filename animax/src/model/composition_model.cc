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

#include "src/model/composition_model.h"

#include "src/layer/text_helper.h"
#include "src/model/layer_model.h"
#include "src/model/text/font_character_model.h"

namespace lynx {
namespace animax {

CompositionModel::CompositionModel(float scale) : scale_(scale) {
  font_asset_manager_ = TextHelper::Impl().CreateFontAssetManager();
  DCHECK(font_asset_manager_);
  ANIMAX_LOGI("Composition model is created, this: " << this);
}

CompositionModel::~CompositionModel() {
  ANIMAX_LOGI("Composition model is destroyed, this: " << this);
}

void CompositionModel::Init(std::unique_ptr<RectF> bounds, float start_frame,
                            float end_frame, float frame_rate, bool enable_3d) {
  bounds_ = std::move(bounds);
  start_frame_ = start_frame;
  end_frame_ = end_frame;
  frame_rate_ = frame_rate;
  enable_3d_ = enable_3d;
}

long CompositionModel::GetDuration() {
  auto duration = (end_frame_ - start_frame_) / frame_rate_ * 1000;
  return duration < 0 ? 0 : duration;
}

void CompositionModel::SetHashDashPattern(bool has_dash_patern) {
  has_dash_pattern_ = has_dash_patern;
}

void CompositionModel::IncrementMatteOrMaskCount(int32_t count) {
  mask_and_matte_count_ += count;
}

MarkerModel* CompositionModel::GetMarker(const std::string& marker_name) {
  for (auto& marker : markers_) {
    if (marker->MatchesName(marker_name)) {
      return marker.get();
    }
  }
  return nullptr;
}

Asset* CompositionModel::GetResource(ResourceType type, const std::string& id) {
  switch (type) {
    case ResourceType::kImage: {
      if (images_.count(id) > 0) {
        return images_[id].get();
      }
      break;
    }
    case ResourceType::kFont: {
      auto& fonts = GetFonts();
      if (fonts.count(id) > 0) {
        return fonts[id].get();
      }
      break;
    }
    case ResourceType::kVideo: {
      if (videos_.count(id) > 0) {
        return videos_[id].get();
      }
      break;
    }
    case ResourceType::kAudio: {
      if (audios_.count(id) > 0) {
        return audios_[id].get();
      }
      break;
    }
    default: {
    }
  }
  return nullptr;
}

}  // namespace animax
}  // namespace lynx
