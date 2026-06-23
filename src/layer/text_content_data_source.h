// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXT_CONTENT_DATA_SOURCE_H_
#define ANIMAX_SRC_LAYER_TEXT_CONTENT_DATA_SOURCE_H_

#include <stdint.h>

#include <cstddef>
#include <string>
#include <vector>

#include "include/base/macros.h"
#include "src/layer/text_layer_animations.h"
#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

class DocumentData;
class FontAsset;
class FontAssetManager;
struct TextLayerAnimations;

class ANIMAX_EXPORT TextContentDataSource {
 public:
  TextContentDataSource(const TextLayerAnimations& animations,
                        FontAssetManager& font_asset_manager);
  ~TextContentDataSource();

  struct RangeStyle {
    size_t segment_start = 0;
    size_t segment_end = 0;
    Color color;
    Color stroke_color;
    float stroke_width = 0.f;
    float skew = 0.f;

    bool HasStyle() const {
      return !color.IsEmpty() || !stroke_color.IsEmpty() ||
             stroke_width != 0.f || skew != 0.f;
    }
  };

  const DocumentData& GetDocumentData() const;
  int32_t GetColor() const;
  int32_t GetStrokeColor() const;
  float GetStrokeWidth() const;
  float GetTracking() const;
  float GetTextSize() const;
  std::string GetFontName() const;
  void* GetFontMgrCollection() const;
  FontAsset* GetFontAsset() const;
  bool GetLayoutOnlyOnce() const;

  // Returns a read-only list of animator properties and callers must NOT mutate
  // any underlying animation objects.
  const std::vector<TextLayerAnimations::AnimatorProperty>&
  GetAnimatorPropertyList() const;

  const std::vector<RangeStyle> GetRangeAnimatorPropertyList(
      size_t text_length) const;

 private:
  const TextLayerAnimations& animations_;
  FontAssetManager& font_asset_manager_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXT_CONTENT_DATA_SOURCE_H_
