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

#include "src/layer/text_layer.h"

#include "base/include/string/string_utils.h"
#include "src/base/log/log.h"
#include "src/layer/text_helper.h"
#include "src/render/canvas.h"
#include "src/resource/asset/font_asset.h"

namespace lynx {
namespace animax {

TextLayer::TextLayer(LayerModel& layer, CompositionModel& composition)
    : BaseLayer(layer, composition) {
  animations_.text_keyframe = layer.GetText()->CreateAnimation();
  AddAnimation(animations_.text_keyframe.get());

  auto* properties_list = layer.GetTextPropertiesList();
  if (properties_list) {
    for (auto& properties : *properties_list) {
      animations_.animator_property_list.emplace_back(
          CreateAnimator(properties.get()));
    }
  }
}

TextLayer::~TextLayer() = default;

void TextLayer::AddAnimationsForRangeSelector(
    RangeSelectorProperty& result_property,
    const AnimatableTextRangeSelector& range_selector) {
  result_property.range_units = range_selector.GetRangeUnits();

  auto* offset = range_selector.GetAnimatableOffset();
  if (offset) {
    result_property.offset = offset->CreateAnimation();
    AddAnimation(result_property.offset.get());
  }
  auto* start = range_selector.GetAnimatableStart();
  if (start) {
    result_property.start = start->CreateAnimation();
    AddAnimation(result_property.start.get());
  }
  auto* end = range_selector.GetAnimatableEnd();
  if (end) {
    result_property.end = end->CreateAnimation();
    AddAnimation(result_property.end.get());
  }
}

TextLayerAnimations::AnimatorProperty TextLayer::CreateAnimator(
    const AnimatableTextProperty* text_property) {
  TextLayerAnimations::AnimatorProperty animator;
  auto* color = text_property->GetColor();
  if (color) {
    animator.color = color->CreateAnimation();
    AddAnimation(animator.color.get());
  }
  auto* stroke = text_property->GetStroke();
  if (stroke) {
    animator.stroke_color = stroke->CreateAnimation();
    AddAnimation(animator.stroke_color.get());
  }
  auto* stroke_width = text_property->GetStrokeWidth();
  if (stroke_width) {
    animator.stroke_width = stroke_width->CreateAnimation();
    AddAnimation(animator.stroke_width.get());
  }
  auto* tracking = text_property->GetTracking();
  if (tracking) {
    animator.tracking = tracking->CreateAnimation();
    AddAnimation(animator.tracking.get());
  }

  auto* skew = text_property->GetSkew();
  if (skew) {
    animator.skew = skew->CreateAnimation();
    AddAnimation(animator.skew.get());
  }

  auto* range_selector = text_property->GetRangeSelector();
  if (range_selector) {
    animator.range_selector = std::make_unique<RangeSelectorProperty>();
    AddAnimationsForRangeSelector(*animator.range_selector, *range_selector);
  }
  return animator;
}

void TextLayer::AddUpdateListenerToRangeSelectorProperty(
    RangeSelectorProperty& range_selector) {
  if (range_selector.offset) {
    range_selector.offset->AddUpdateListener(this);
  }
  if (range_selector.start) {
    range_selector.start->AddUpdateListener(this);
  }
  if (range_selector.end) {
    range_selector.end->AddUpdateListener(this);
  }
}

void TextLayer::AddUpdateListenerToAnimatorProperty(
    TextLayerAnimations::AnimatorProperty& animator) {
  if (animator.color) {
    animator.color->AddUpdateListener(this);
  }
  if (animator.stroke_color) {
    animator.stroke_color->AddUpdateListener(this);
  }
  if (animator.stroke_width) {
    animator.stroke_width->AddUpdateListener(this);
  }
  if (animator.tracking) {
    animator.tracking->AddUpdateListener(this);
  }
  if (animator.skew) {
    animator.skew->AddUpdateListener(this);
  }
  if (animator.range_selector) {
    AddUpdateListenerToRangeSelectorProperty(*animator.range_selector);
  }
}

void TextLayer::CheckFontAsset() {
#ifdef OS_WASM
  // Lazy font check - only execute once
  if (font_checked_ || !data_source_) {
    return;
  }
  font_checked_ = true;
  auto* font_asset = data_source_->GetFontAsset();
  if (font_asset) {
    auto* font = font_asset->GetFont();
    if (font && font->HasValidTypeface()) {
      return;
    }
  }
  if (!Font::HasDefaultTypeface()) {
    if (auto listener = weak_listener_.lock()) {
      listener->OnLayerError(EventError::kDefaultFontNotFound,
                             "No default font available");
    } else {
      ANIMAX_LOGE(
          "No default font available. Please register a default (fallback) "
          "font.");
    }
  } else {
    if (auto listener = weak_listener_.lock()) {
      listener->OnLayerWarning(EventWarning::kTextLayerFontInvalid,
                               "Text layer font is invalid");
    } else {
      ANIMAX_LOGW("Text layer font is invalid. Please check the font asset.");
    }
  }
#endif
}

void TextLayer::Init() {
  BaseLayer::Init();

  if (GetBlurEffect()) {
    blur_element_ = std::make_unique<BlurElement>(*this);
    blur_element_->Init();
  }
  data_source_ = std::make_unique<TextContentDataSource>(
      animations_, composition_.GetFontAssetManager());

  text_content_ = TextHelper::Impl().CreateTextContent(*data_source_);
  text_content_->Init(this);

  if (animations_.text_keyframe) {
    animations_.text_keyframe->AddUpdateListener(this);
  }

  for (auto& animator_property : animations_.animator_property_list) {
    AddUpdateListenerToAnimatorProperty(animator_property);
  }
}

void TextLayer::DrawLayer(Canvas& canvas, Matrix& matrix, int32_t alpha) {
  if (!text_content_) {
    return;
  }
  CheckFontAsset();
  canvas.Save();
  canvas.Concat(matrix);
  text_content_->Draw(canvas, alpha);
  canvas.Restore();
}

void TextLayer::ApplyEffects(Paint& paint) {
  if (blur_element_) {
    blur_element_->Draw(paint, *this, false);
  }
}

void TextLayer::GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                          bool apply_parent) {
  CheckFontAsset();
  BaseLayer::GetBounds(out_bounds, parent_matrix, apply_parent);
  auto& bounds = composition_.GetBounds();
  out_bounds.Set(0, 0, bounds.GetWidth(), bounds.GetHeight());
  if (text_content_ && text_content_->GetRect(out_bounds)) {
    bounds_matrix_->MapRect(out_bounds);
  }
}

inline KeyframeAnimation* TextLayer::GetAnimationFromAnimatorProperty(
    const TextLayerAnimations::AnimatorProperty& animator,
    LayerPropertyType type) {
  switch (type) {
    case LayerPropertyType::kTextColor:
    case LayerPropertyType::kColor:
      return animator.color.get();
    case LayerPropertyType::kStrokeColor:
      return animator.stroke_color.get();
    case LayerPropertyType::kStrokeWidth:
      return animator.stroke_width.get();
    case LayerPropertyType::kTextTracking:
      return animator.tracking.get();
    case LayerPropertyType::kTextSkew:
      return animator.skew.get();
    case LayerPropertyType::kTextSize:
      return animator.text_size_callback.get();
    case LayerPropertyType::kTextRangeOffset:
      return animator.range_selector ? animator.range_selector->offset.get()
                                     : nullptr;
    case LayerPropertyType::kTextRangeStart:
      return animator.range_selector ? animator.range_selector->start.get()
                                     : nullptr;
    case LayerPropertyType::kTextRangeEnd:
      return animator.range_selector ? animator.range_selector->end.get()
                                     : nullptr;
    default:
      return nullptr;
  }
}

TextLayerAnimations::AnimatorProperty& TextLayer::AnimatorPropertyForType(
    LayerPropertyType type) {
  auto& animator_property_list = animations_.animator_property_list;
  if (animator_property_list.empty()) {
    animator_property_list.emplace_back();
  } else {
    for (auto& animator : animator_property_list) {
      if (GetAnimationFromAnimatorProperty(animator, type) != nullptr) {
        return animator;
      }
    }
  }
  return animator_property_list.front();
}

KeyframeAnimation* TextLayer::GetAnimationForProperty(LayerPropertyType type) {
  auto& layer_ref = *this;
  switch (type) {
    case LayerPropertyType::kTextColor:
    case LayerPropertyType::kColor:
      return GetOrCreateAnimation<ColorKeyframeAnimation>(
          AnimatorPropertyForType(type).color, layer_ref, this);
    case LayerPropertyType::kStrokeColor:
      return GetOrCreateAnimation<ColorKeyframeAnimation>(
          AnimatorPropertyForType(type).stroke_color, layer_ref, this);
    case LayerPropertyType::kStrokeWidth:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(
          AnimatorPropertyForType(type).stroke_width, layer_ref, this);
    case LayerPropertyType::kTextTracking:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(
          AnimatorPropertyForType(type).tracking, layer_ref, this);
    case LayerPropertyType::kTextSkew:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(
          AnimatorPropertyForType(type).skew, layer_ref, this);
    case LayerPropertyType::kTextSize:
      return GetOrCreateAnimation<FloatKeyframeAnimation>(
          AnimatorPropertyForType(type).text_size_callback, layer_ref, this);
    case LayerPropertyType::kTextValue:
      return GetOrCreateAnimation<TextKeyframeAnimation>(
          animations_.text_keyframe, layer_ref, this);
    case LayerPropertyType::kTextRangeOffset: {
      auto& animator = AnimatorPropertyForType(type);
      if (animator.range_selector) {
        return GetOrCreateAnimation<FloatKeyframeAnimation>(
            animator.range_selector->offset, layer_ref, this);
      }
      return BaseLayer::GetAnimationForProperty(type);
    }
    case LayerPropertyType::kTextRangeStart: {
      auto& animator = AnimatorPropertyForType(type);
      if (animator.range_selector) {
        return GetOrCreateAnimation<FloatKeyframeAnimation>(
            animator.range_selector->start, layer_ref, this);
      }
      return BaseLayer::GetAnimationForProperty(type);
    }
    case LayerPropertyType::kTextRangeEnd: {
      auto& animator = AnimatorPropertyForType(type);
      if (animator.range_selector) {
        return GetOrCreateAnimation<FloatKeyframeAnimation>(
            animator.range_selector->end, layer_ref, this);
      }
      return BaseLayer::GetAnimationForProperty(type);
    }
    default:
      return BaseLayer::GetAnimationForProperty(type);
  }
}

}  // namespace animax
}  // namespace lynx
