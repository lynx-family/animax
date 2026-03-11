// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_ASSET_TASK_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_ASSET_TASK_H_

#include <vector>

#include "include/resource/loader_error.h"
#include "include/resource/uri_info.h"
#include "src/resource/asset/asset.h"

namespace lynx {
namespace animax {

class CompositionModel;

struct CompositionAssetRequest {
  MainResourceUriInfo main_uri_info;
  std::shared_ptr<CompositionModel> model;
};

struct AssetResponse {
  ResourceType type;
  std::string id;
  LoaderError error;
};

struct CompositionAssetResponse {
  std::shared_ptr<CompositionModel> model;
  std::vector<AssetResponse> asset_responses;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_ASSET_TASK_H_
