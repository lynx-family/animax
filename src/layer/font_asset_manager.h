// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_FONT_ASSET_MANAGER_H_
#define ANIMAX_SRC_LAYER_FONT_ASSET_MANAGER_H_

#include <memory>
#include <unordered_map>

#include "src/base/thread/thread_assert.h"
#include "src/resource/asset/font_asset.h"

namespace lynx {
namespace animax {

using FontAssetMap =
    std::unordered_map<std::string, std::shared_ptr<FontAsset>>;

class FontAssetManager {
 public:
  virtual ~FontAssetManager() = default;

  FontAssetMap& GetFontAssetMap() { return font_asset_map_; }

  virtual void* GetFontMgrCollection() {
    ThreadAssert::Assert(ThreadAssert::Type::kGPU);
    return nullptr;
  }

  virtual std::string RedirectNameWithSuffix(
      const std::string& font_name) const {
    return font_name;
  }

 protected:
  FontAssetMap font_asset_map_;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_FONT_ASSET_MANAGER_H_
