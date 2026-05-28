// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXT_CONTENT_DATA_SOURCE_H_
#define ANIMAX_SRC_LAYER_TEXT_CONTENT_DATA_SOURCE_H_

#include <stdint.h>

#include <string>

#include "include/base/macros.h"

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

  const DocumentData& GetDocumentData() const;
  int32_t GetColor() const;
  int32_t GetStrokeColor() const;
  float GetStrokeWidth() const;
  float GetTracking() const;
  float GetSkew() const;
  float GetTextSize() const;
  std::string GetFontName() const;
  void* GetFontMgrCollection() const;
  FontAsset* GetFontAsset() const;
  bool GetLayoutOnlyOnce() const;

 private:
  const TextLayerAnimations& animations_;
  FontAssetManager& font_asset_manager_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXT_CONTENT_DATA_SOURCE_H_
