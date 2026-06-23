// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/text_content_data_source.h"

#include <algorithm>
#include <utility>

#include "src/layer/font_asset_manager.h"
#include "src/layer/text_layer_animations.h"
#include "src/layer/text_range_selector_property.h"

namespace lynx {
namespace animax {
namespace {

void ApplyRangeStyle(const TextLayerAnimations::AnimatorProperty& property,
                     TextContentDataSource::RangeStyle& range_style) {
  if (property.color) {
    range_style.color = property.color->GetValue();
  }
  if (property.stroke_color) {
    range_style.stroke_color = property.stroke_color->GetValue();
  }
  if (property.stroke_width) {
    range_style.stroke_width = property.stroke_width->GetValue().Get();
  }
  if (property.skew) {
    range_style.skew = property.skew->GetValue().Get();
  }
}

}  // namespace

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
  const auto& animator_property_list = GetAnimatorPropertyList();
  for (auto it = animator_property_list.rbegin();
       it != animator_property_list.rend(); ++it) {
    if (it->color) {
      return it->color->GetValue().GetInt();
    }
  }
  return GetDocumentData().GetColor();
}
int32_t TextContentDataSource::GetStrokeColor() const {
  const auto& animator_property_list = GetAnimatorPropertyList();
  for (auto it = animator_property_list.rbegin();
       it != animator_property_list.rend(); ++it) {
    if (it->stroke_color) {
      return it->stroke_color->GetValue().GetInt();
    }
  }
  return GetDocumentData().GetStrokeColor();
}

float TextContentDataSource::GetStrokeWidth() const {
  const auto& animator_property_list = GetAnimatorPropertyList();
  for (auto it = animator_property_list.rbegin();
       it != animator_property_list.rend(); ++it) {
    if (it->stroke_width) {
      return it->stroke_width->GetValue().Get();
    }
  }
  return GetDocumentData().GetStrokeWidth();
}

float TextContentDataSource::GetTracking() const {
  constexpr float tracking_scale = 10.f;
  float tracking = GetDocumentData().GetTracking() / tracking_scale;
  for (const auto& animator_property : GetAnimatorPropertyList()) {
    if (animator_property.tracking) {
      tracking += animator_property.tracking->GetValue().Get();
    }
  }
  return tracking;
}

float TextContentDataSource::GetTextSize() const {
  const auto& animator_property_list = GetAnimatorPropertyList();
  for (auto it = animator_property_list.rbegin();
       it != animator_property_list.rend(); ++it) {
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

const std::vector<TextLayerAnimations::AnimatorProperty>&
TextContentDataSource::GetAnimatorPropertyList() const {
  return animations_.animator_property_list;
}

const std::vector<TextContentDataSource::RangeStyle>
TextContentDataSource::GetRangeAnimatorPropertyList(size_t text_length) const {
  std::vector<size_t> boundaries = {0, text_length};

  for (const auto& property : GetAnimatorPropertyList()) {
    size_t range_start = 0;
    size_t range_length = text_length;
    if (property.range_selector) {
      range_start = property.range_selector->GetRangeStart(text_length);
      range_length = property.range_selector->GetRangeLength(text_length);
    }
    const size_t range_end =
        range_start >= text_length
            ? text_length
            : range_start + std::min(range_length, text_length - range_start);
    if (range_start >= range_end) {
      continue;
    }
    boundaries.push_back(range_start);
    boundaries.push_back(range_end);
  }

  std::sort(boundaries.begin(), boundaries.end());
  boundaries.erase(std::unique(boundaries.begin(), boundaries.end()),
                   boundaries.end());

  std::vector<RangeStyle> range_styles;
  for (size_t i = 1; i < boundaries.size(); ++i) {
    const size_t segment_start = boundaries[i - 1];
    const size_t segment_end = boundaries[i];
    if (segment_start >= segment_end) {
      continue;
    }

    RangeStyle range_style;
    range_style.segment_start = segment_start;
    range_style.segment_end = segment_end;
    for (const auto& property : GetAnimatorPropertyList()) {
      size_t range_start = 0;
      size_t range_length = text_length;
      if (property.range_selector) {
        range_start = property.range_selector->GetRangeStart(text_length);
        range_length = property.range_selector->GetRangeLength(text_length);
      }
      const size_t range_end =
          range_start >= text_length
              ? text_length
              : range_start + std::min(range_length, text_length - range_start);
      if (range_start >= segment_end || segment_start >= range_end) {
        continue;
      }
      ApplyRangeStyle(property, range_style);
    }
    if (range_style.HasStyle()) {
      range_styles.push_back(range_style);
    }
  }
  return range_styles;
}

}  // namespace animax
}  // namespace lynx
