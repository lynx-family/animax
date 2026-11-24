// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_PRELOADED_ASSET_LOADER_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_PRELOADED_ASSET_LOADER_H_

#include <functional>
#include <memory>
#include <vector>

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/resource/asset/audio_asset.h"
#include "src/resource/asset/video_asset.h"
#include "src/resource/composition_loader/composition_asset_task.h"

namespace lynx {
namespace animax {

enum class PreloadedAssetStage { kLoad, kPrepare };

struct PreloadedAssetRequest {
  ResourceType type;
  ResourceRequest request;
};

struct PreloadedAssetResponse {
  std::unique_ptr<Bitmap> bitmap;
  std::unique_ptr<RawData> font_data;
  std::shared_ptr<AudioAsset> audio;
  std::shared_ptr<VideoAsset> video;
  int64_t payload_bytes = 0;
  AssetResponse result{};
  PreloadedAssetStage stage = PreloadedAssetStage::kLoad;
};

// Uses the host's existing resource service. Preparation and completion run on
// the Resource thread. Every attempt owns a fresh candidate; failure never
// exposes partially prepared data. The caller checks its own host/Env lifetime
// before publishing the result to JS.
ANIMAX_EXPORT void LoadPreloadedAsset(
    ResourceType type, ResourceRequest request,
    Loader<ResourceRequest, ResourceResponse>::Ptr loader,
    std::function<void(PreloadedAssetResponse)> callback);

// Results preserve request order. Completion runs on the Resource thread,
// including when there are no requests.
ANIMAX_EXPORT void LoadPreloadedAssets(
    std::vector<PreloadedAssetRequest> requests,
    Loader<ResourceRequest, ResourceResponse>::Ptr loader,
    std::function<void(std::vector<PreloadedAssetResponse>)> callback);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_PRELOADED_ASSET_LOADER_H_
