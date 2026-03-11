// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXTRA_FONT_ASSET_MANAGER_TEXTRA_H_
#define ANIMAX_SRC_LAYER_TEXTRA_FONT_ASSET_MANAGER_TEXTRA_H_

#include "src/layer/font_asset_manager.h"

namespace lynx {
namespace animax {

class FontAssetManagerTextra : public FontAssetManager {
 public:
  FontAssetManagerTextra();
  ~FontAssetManagerTextra() override;

  void* GetFontMgrCollection() override;

  std::string RedirectNameWithSuffix(
      const std::string& font_name) const override;

 private:
  std::string asset_font_name_suffix_;
  std::unique_ptr<void, void (*)(void*)> font_mgr_collection_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXTRA_FONT_ASSET_MANAGER_TEXTRA_H_
