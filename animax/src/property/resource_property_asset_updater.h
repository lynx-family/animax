// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_RESOURCE_PROPERTY_ASSET_UPDATER_H_
#define ANIMAX_SRC_PROPERTY_RESOURCE_PROPERTY_ASSET_UPDATER_H_

#include "src/property/property_update_context.h"
#include "src/resource/asset/asset.h"

namespace lynx {
namespace animax {
class ResourcePropertyAssetUpdator : public AssetVisitor {
 public:
  explicit ResourcePropertyAssetUpdator(PropertyUpdateContext& context)
      : context_{context} {}

  ~ResourcePropertyAssetUpdator() override = default;
  void Visit(FontAsset& asset) override;
  void Visit(ImageAsset& asset) override;
  void Visit(VideoAsset& asset) override;
  void Visit(AudioAsset& asset) override;

 private:
  PropertyUpdateContext& context_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_RESOURCE_PROPERTY_ASSET_UPDATER_H_
