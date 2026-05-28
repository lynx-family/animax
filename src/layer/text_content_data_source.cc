// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/text_content_data_source.h"

#include "src/layer/font_asset_manager.h"
#include "src/layer/text_layer_animations.h"

namespace lynx {
namespace animax {

TextContentDataSource::TextContentDataSource(
    const TextLayerAnimations& animations, FontAssetManager& font_asset_manager)
    : animations_(animations), font_asset_manager_(font_asset_manager) {}

TextContentDataSource::~TextContentDataSource() = default;

const DocumentData& TextContentDataSource::GetDocumentData() const {
  DCHECK(animations_.text_keyframe);
  return animations_.text_keyframe->GetValue();
}
void* TextContentDataSource::GetFontMgrCollection() const {
  return font_asset_manager_.GetFontMgrCollection();
}

int32_t TextContentDataSource::GetColor() const {
  for (auto it = animations_.animator_property_list.rbegin();
       it != animations_.animator_property_list.rend(); ++it) {
    if (it->color) {
      return it->color->GetValue().GetInt();
    }
  }
  return GetDocumentData().GetColor();
}
int32_t TextContentDataSource::GetStrokeColor() const {
  for (auto it = animations_.animator_property_list.rbegin();
       it != animations_.animator_property_list.rend(); ++it) {
    if (it->stroke_color) {
      return it->stroke_color->GetValue().GetInt();
    }
  }
  return GetDocumentData().GetStrokeColor();
}

float TextContentDataSource::GetStrokeWidth() const {
  for (auto it = animations_.animator_property_list.rbegin();
       it != animations_.animator_property_list.rend(); ++it) {
    if (it->stroke_width) {
      return it->stroke_width->GetValue().Get();
    }
  }
  return GetDocumentData().GetStrokeWidth();
}

float TextContentDataSource::GetTracking() const {
  constexpr float tracking_scale = 10.f;
  float tracking = GetDocumentData().GetTracking() / tracking_scale;
  for (const auto& animator_property : animations_.animator_property_list) {
    if (animator_property.tracking) {
      tracking += animator_property.tracking->GetValue().Get();
    }
  }
  return tracking;
}

float TextContentDataSource::GetSkew() const {
  for (auto it = animations_.animator_property_list.rbegin();
       it != animations_.animator_property_list.rend(); ++it) {
    if (it->skew) {
      return it->skew->GetValue().Get();
    }
  }
  return 0.f;
}

float TextContentDataSource::GetTextSize() const {
  for (auto it = animations_.animator_property_list.rbegin();
       it != animations_.animator_property_list.rend(); ++it) {
    if (it->text_size_callback) {
      return it->text_size_callback->GetValue().Get();
    }
  }
  return GetDocumentData().GetSize();
}

std::string TextContentDataSource::GetFontName() const {
  auto font_name = GetDocumentData().GetFontName();
  return font_asset_manager_.RedirectNameWithSuffix(font_name);
}

FontAsset* TextContentDataSource::GetFontAsset() const {
  auto font_name = GetDocumentData().GetFontName();
  auto& map = font_asset_manager_.GetFontAssetMap();
  auto it = map.find(font_name);
  return it != map.end() ? it->second.get() : nullptr;
}

bool TextContentDataSource::GetLayoutOnlyOnce() const {
  const int32_t key_frame_size = animations_.text_keyframe->GetSize();
  return key_frame_size == 0 ||
         (key_frame_size == 1 &&
          animations_.text_keyframe->GetKeyframe(0)->IsStatic());
}

}  // namespace animax
}  // namespace lynx
