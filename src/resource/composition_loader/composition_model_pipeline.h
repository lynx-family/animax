// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_MODEL_PIPELINE_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_MODEL_PIPELINE_H_

#include <memory>

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "include/resource/unzip_task.h"
#include "include/resource/uri_info.h"
#include "src/base/monitor/metrics_data_source.h"
#include "src/base/monitor/timestamp_recorder.h"
#include "src/base/thread/task_runner.h"
#include "src/model/composition_model.h"
#include "src/resource/composition_loader/composition_asset_task.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/resource_loader_listener.h"

namespace lynx {
namespace animax {

using ResourceLoader = Loader<ResourceRequest, ResourceResponse>;
using UnzipLoader = Loader<UnzipRequest, UnzipResponse>;
using ModelLoader = Loader<CompositionModelRequest, CompositionModelResponse>;
using AssetLoader = Loader<CompositionAssetRequest, CompositionAssetResponse>;

/**
 * @class CompositionModelPipeline
 *
 * @brief Manages the composition model loading and asset handling.
 *
 * This class is responsible for managing the loading and initialization of
 * composition models and their associated assets from various sources. This
 * class ensures thread safety by executing certain set operations on the main
 * thread and guarantees that all callback operations are also triggered on the
 * main thread.
 *
 * Usage:
 * - To use the `CompositionModelPipeline`, first create an instance using the
 * constructor.
 * - Initialize the pipeline with appropriate loaders and listeners by calling
 * the `Init()` method.
 * - Configuration methods such as `SetSrcPolyfill`, `SetImageFolder`, and
 * `SetHasDynamicResource` allow for customization of the loading process.
 * - Use `LoadCompositionModelFromURI`, `LoadCompositionModelFromJSONString`, or
 * `LoadCompositionModelAsset` to load models and their assets.
 *
 *
 * @note
 * - All Load functions will returns error in their callback before calling
 * Init() with valid loaders.
 * - All operations that modify the state of the pipeline are executed on the
 * main thread to ensure thread safety.
 */

class CompositionModelPipeline
    : public std::enable_shared_from_this<CompositionModelPipeline>,
      public ResourceLoaderListener,
      public MetricsDataSource {
 public:
  CompositionModelPipeline() = default;

  /**
   * Initializes the pipeline with necessary loaders.
   * This function is thread-safe.
   *
   * @param resource_loader Pointer to the ResourceLoader.
   * @param unzip_loader Pointer to the UnzipLoader.
   */
  void Init(ResourceLoader::Ptr resource_loader, UnzipLoader::Ptr unzip_loader);

  /**
   * Sets the source polyfill map for rewriting "%s" path in JSON.
   * This function is thread-safe.
   *
   * @param polyfill A map of string replacements used for source URL
   * modification.
   */
  void SetSrcPolyfill(std::unordered_map<std::string, std::string> polyfill);

  /**
   * Set and override the `dir_name` in @class ImageAssetModel for all images
   * assets. This function is thread-safe.
   *
   * @param image_folder The path to the image folder.
   */
  void SetImageFolder(std::string image_folder);

  /**
   * Configures whether the pipeline is handling with a composition model with
   * dynamic resources. This function is thread-safe. If true, then the pipeline
   * will skip loading assets for composition models.
   *
   * @param has_dynamic_resource True to for having dynamic resources, false
   * otherwise.
   */
  void SetHasDynamicResource(bool has_dynamic_resource);

  /**
   * Configures whether the pipeline will handle audio assets in a composition
   * model. This function is thread-safe. If true, then the pipeline will skip
   * parsing audio assets.
   *
   * @param enable True to handle audio assets in this pipeline.
   */
  void SetEnableAudio(bool enable);

  /**
   * Configures whether a main resource URI without a file extension should be
   * treated as JSON.
   *
   * @param allow True to treat extensionless main resource URIs as JSON.
   */
  void SetAllowExtensionlessJson(bool allow);

  /**
   * Initiates the loading of a composition model from a specified URI.
   *
   * @param uri The URI from which to load the composition model.
   * @param scale The scale factor to be applied to the assets.
   * @param callback The callback to be invoked upon successful or failed
   * loading. Guaranteed to be called on the main thread.
   */
  void LoadCompositionModelFromURI(std::string uri, float scale,
                                   AssetLoader::CallbackType callback);

  /**
   * Initiates the loading of a composition model from a JSON string.
   *
   * @param json_str The JSON string containing the model data.
   * @param scale The scale factor to be applied to the assets.
   * @param callback The callback to be invoked upon successful or failed
   * loading. Guaranteed to be called on the main thread.
   */
  void LoadCompositionModelFromJSONString(std::string json_str, float scale,
                                          AssetLoader::CallbackType callback);

  /**
   * Initiates the loading of assets for a given composition model.
   *
   * @param model Shared pointer to the CompositionModel for which assets need
   * to be loaded.
   * @param callback The callback to be invoked upon successful or failed
   * loading. Guaranteed to be called on the main thread.
   */
  void LoadCompositionModelAsset(std::shared_ptr<CompositionModel> model,
                                 AssetLoader::CallbackType callback);

  void OnResourceLoaderTraceEvent(TraceEventType trace_event,
                                  std::string main_uri = {},
                                  std::string asset_id = {}) override;
  void OnBeforeAssetsLoad(
      const std::vector<std::shared_ptr<Asset>>& assets,
      const std::vector<std::shared_ptr<Asset>>& invalid_assets) override;

  void Trace(TraceEventType type) override;

  TimestampArray ExportTimestamps() const override;

 private:
  ModelLoader::Ptr ChooseLoaderForCompositionModelRequest(
      const CompositionModelRequest& request);
  AssetLoader::CallbackType MakeCallback(AssetLoader::CallbackType);
  bool ShouldLoadAssetAfterLoadingCompositionModel(
      const CompositionModelRequest& request);
  void LoadCompositionModel(CompositionModelRequest request,
                            AssetLoader::CallbackType callback);

  bool has_dynamic_resource_{false};
  bool enable_audio_{false};
  bool allow_extensionless_json_{false};
  MainResourceUriInfo main_uri_info_{};
  ModelLoader::Ptr json_composition_model_loader_{};
  ModelLoader::Ptr zip_composition_model_loader_{};
  ModelLoader::Ptr json_alpha_video_model_loader_{};
  AssetLoader::Ptr composition_asset_loader_{};
  fml::RefPtr<fml::TaskRunner> task_runner_{};
  TimestampRecorder resource_thread_recorder_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_MODEL_PIPELINE_H_
