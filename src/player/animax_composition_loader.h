// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANIMAX_COMPOSITION_LOADER_H_
#define ANIMAX_SRC_PLAYER_ANIMAX_COMPOSITION_LOADER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/include/lynx_actor.h"
#include "include/resource/loader.h"
#include "src/base/monitor/metrics_data_source.h"
#include "src/model/composition_model.h"
#include "src/resource/composition_loader/composition_asset_task.h"
#include "src/resource/composition_loader/composition_model_pipeline.h"

namespace lynx {
namespace animax {

class Asset;

/**
 * Thread-safe composition loader via LynxActor pattern.
 *
 * AnimaXCompositionLoader provides a thread-safe interface for loading
 * composition models and assets. ALL operations are restricted to the resource
 * thread through the LynxActor pattern, ensuring thread safety and proper
 * resource management.
 *
 * ## Basic Usage:
 * // 1. Create loader actor
 * auto loader_actor = AnimaXCompositionLoader::Create();
 *
 * // 2. Initialize with resource and unzip loaders
 * loader_actor->Act([resource_loader, unzip_loader](auto& loader) {
 *   loader->Init(resource_loader, unzip_loader);
 * });
 *
 * // 3. Load composition from URI
 * loader_actor->Act([uri, scale](auto& loader) {
 *   loader->LoadCompositionModelFromURI(uri, scale,
 *     [](CompositionAssetResponse res, LoaderError error) {
 *       if (res.model && !error) {
 *         // Handle successful loading
 *       } else {
 *         // Handle error
 *       }
 *     });
 * });
 *
 * All loader operations are executed on the resource thread. Callbacks are
 * invoked asynchronously and should handle thread-safety appropriately.
 */
class AnimaXCompositionLoader : public MetricsDataSource {
 public:
  ~AnimaXCompositionLoader() = default;

  static std::shared_ptr<shell::LynxActor<AnimaXCompositionLoader>> Create();

  void Init(ResourceLoader::Ptr resource_loader, UnzipLoader::Ptr unzip_loader);

  void SetSrcPolyfill(std::unordered_map<std::string, std::string> polyfill);

  void SetImageFolder(std::string image_folder);

  void SetHasDynamicResource(bool has_dynamic_resource);

  void SetEnableAudio(bool enable);

  void LoadCompositionModelFromURI(std::string uri, float scale,
                                   AssetLoader::CallbackType callback);

  void LoadCompositionModelFromJSONString(std::string json_str, float scale,
                                          AssetLoader::CallbackType callback);

  void LoadCompositionModelAsset(std::shared_ptr<CompositionModel> model,
                                 AssetLoader::CallbackType callback);

  void Trace(TraceEventType type) override;

  TimestampArray ExportTimestamps() const override;

 private:
  AnimaXCompositionLoader()
      : pipeline_(std::make_shared<CompositionModelPipeline>()) {}

  // Use shared_ptr rather than unique_ptr to use weak_from_this in pipeline.
  std::shared_ptr<CompositionModelPipeline> pipeline_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANIMAX_COMPOSITION_LOADER_H_
