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

#include "src/layer/image_layer.h"

namespace lynx {
namespace animax {

ImageLayer::ImageLayer(LayerModel& layer_model, CompositionModel& composition)
    : BaseLayer(layer_model, composition),
      paint_(std::make_unique<Paint>()),
      scale_(composition.GetScale()) {
  auto id = layer_model_.GetRefId();
  image_asset_ = composition.GetImages()[id];
  paint_->SetAntiAlias(true);
}

void ImageLayer::Init() {
  BaseLayer::Init();
  if (GetBlurEffect()) {
    blur_element_ = std::make_unique<BlurElement>(*this);
    blur_element_->Init();
  }
}

void ImageLayer::GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                           bool apply_parent) {
  BaseLayer::GetBounds(out_bounds, parent_matrix, apply_parent);
  if (image_asset_) {
    out_bounds.Set(0, 0, image_asset_->Model().width * scale_,
                   image_asset_->Model().height * scale_);
    bounds_matrix_->MapRect(out_bounds);
  }
}

void ImageLayer::DrawLayer(Canvas& canvas, Matrix& parent_matrix,
                           int32_t parent_alpha) {
  auto image = GetImage(canvas.GetRealContext());
  if (image == nullptr) {
    return;
  }

  paint_->SetAlpha(parent_alpha);

  if (color_filter_animation_) {
    paint_->SetColorFilter(color_filter_animation_->GetValue());
  }

  if (blur_element_) {
    blur_element_->Draw(*paint_, *this, false);
  }

  canvas.Save();
  canvas.Concat(parent_matrix);
  src_.Set(0, 0, static_cast<int>(image->GetWidth()),
           static_cast<int>(image->GetHeight()));
  dst_.Set(0, 0, static_cast<int>(image_asset_->Model().width * scale_),
           static_cast<int>(image_asset_->Model().height * scale_));

  canvas.DrawImageRect(*image, src_, dst_, *paint_);
  canvas.Restore();
}

Image* ImageLayer::GetImage(RealContext* real_context) {
  if (image_asset_) {
    return image_asset_->GetImage(real_context);
  }

  return nullptr;
}

KeyframeAnimation* ImageLayer::GetAnimationForProperty(LayerPropertyType type) {
  switch (type) {
    case LayerPropertyType::kColorFilter: {
      return GetOrCreateAnimation<ColorFilterKeyframeAnimation>(
          color_filter_animation_, *this, this);
    }
    default: {
      return BaseLayer::GetAnimationForProperty(type);
    }
  }
}

}  // namespace animax
}  // namespace lynx
