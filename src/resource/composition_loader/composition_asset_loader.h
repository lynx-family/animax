// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_ASSET_LOADER_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_ASSET_LOADER_H_

#include <memory>

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/resource/composition_loader/composition_asset_task.h"
#include "src/resource/loader/all_settled_loader.h"
#include "src/resource/resource_loader_listener.h"

namespace lynx {
namespace animax {

class CompositionAssetLoader
    : public Loader<CompositionAssetRequest, CompositionAssetResponse> {
 public:
  CompositionAssetLoader(
      Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
      std::weak_ptr<ResourceLoaderListener> listener = {});

  ~CompositionAssetLoader() override = default;
  void Load(CompositionAssetRequest, CallbackType) override;

 private:
  AllSettledLoader<ResourceRequest, ResourceResponse>::Ptr resource_loader_;
  std::weak_ptr<ResourceLoaderListener> listener_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_ASSET_LOADER_H_
