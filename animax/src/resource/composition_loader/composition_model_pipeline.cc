// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/resource/composition_loader/composition_model_pipeline.h"

#include "include/resource/loader_error.h"
#include "include/resource/uri_info.h"
#include "src/model/composition_model.h"
#include "src/resource/composition_loader/composition_asset_loader.h"
#include "src/resource/composition_loader/composition_asset_task.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/composition_loader/json_alpha_video_model_loader.h"
#include "src/resource/composition_loader/json_composition_model_loader.h"
#include "src/resource/composition_loader/zip_composition_model_loader.h"
#include "src/resource/loader/exec_loader.h"
#include "src/resource/log_util.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

void CompositionModelPipeline::Init(ResourceLoader::Ptr resource_loader,
                                    UnzipLoader::Ptr unzip_loader) {
  auto resource_thread_resource_loader =
      ResourceThreadExecLoader(resource_loader);
  json_composition_model_loader_ =
      ResourceThreadExecLoader(JSONCompositionModelLoader::MakeLoader(
          resource_thread_resource_loader, weak_from_this()));
  zip_composition_model_loader_ =
      ResourceThreadExecLoader(ZipCompositionModelLoader::MakeLoader(
          resource_thread_resource_loader, unzip_loader, weak_from_this()));
  json_alpha_video_model_loader_ =
      ResourceThreadExecLoader(JSONAlphaVideoModelLoader::MakeLoader(
          resource_thread_resource_loader, weak_from_this()));
  composition_asset_loader_ = ResourceThreadExecLoader(
      CompositionAssetLoader::Make<CompositionAssetLoader>(
          resource_thread_resource_loader, weak_from_this()));
}

AssetLoader::CallbackType CompositionModelPipeline::MakeCallback(
    AssetLoader::CallbackType callback) {
  return AssetLoader::CallbackType{
      [callback = std::move(callback), weak_self = weak_from_this()](
          auto result, auto error) mutable {
        if (auto self = weak_self.lock(); callback) {
          if (callback) {
            callback(std::move(result), std::move(error));
          }
        }
      }};
}

ModelLoader::Ptr
CompositionModelPipeline::ChooseLoaderForCompositionModelRequest(
    const CompositionModelRequest& request) {
  if (request.uri_info.content_type == UriInfo::ContentType::kZip) {
    return zip_composition_model_loader_;
  } else {
    if (request.uri_info.scheme == UriInfo::Scheme::kFile &&
        IsAlphaVideoDirectory(request.uri_info.uri)) {
      // If the uri is a file scheme and is the alpha video directory,
      // we need to process it with alpha video model loader.
      return json_alpha_video_model_loader_;
    } else {
      return json_composition_model_loader_;
    }
  }
}

void CompositionModelPipeline::LoadCompositionModel(
    CompositionModelRequest request,
    CompositionAssetLoader::CallbackType callback) {
  auto exec_callback = MakeCallback(std::move(callback));
  auto loader = ChooseLoaderForCompositionModelRequest(request);
  if (!loader) {
    exec_callback(
        {}, LoaderError{.code = LoaderErrorCode::kInvalidLoader,
                        .message = "Composition model loader is not ready."});
    return;
  }

  loader->Load(
      std::move(request),
      [weak_self = weak_from_this(), exec_callback = std::move(exec_callback),
       has_dynamic_resource = has_dynamic_resource_](
          CompositionModelResponse res, LoaderError error) mutable {
        auto self = weak_self.lock();
        if (!self) {
          return;
        }

        if (error) {
          exec_callback({}, error);
          return;
        }

        self->main_uri_info_.base_uri = res.base_uri;
        self->main_uri_info_.main_uri = res.main_uri;
        res.model->should_reset_identity_ = !has_dynamic_resource;
        // we assume that animations of type AlphaVideo will be
        // passed in as zip files.
        if (has_dynamic_resource ||
            res.model_type == CompositionModelType::kAlphaVideo) {
          exec_callback(
              CompositionAssetResponse{
                  .model = std::move(res.model),
              },
              {});
          return;
        }

        self->LoadCompositionModelAsset(res.model, std::move(exec_callback));
      });
}

void CompositionModelPipeline::LoadCompositionModelFromURI(
    std::string uri, float scale,
    CompositionAssetLoader::CallbackType callback) {
  auto request = CompositionModelRequest{
      .uri_info = UriInfo{.scheme = ParseUriScheme(uri),
                          .content_type = ParseUriMainResourceContentType(uri),
                          .uri = std::move(uri)},
      .scale = scale,
      .enable_audio = enable_audio_,
  };
  LoadCompositionModel(std::move(request), std::move(callback));
}

void CompositionModelPipeline::LoadCompositionModelFromJSONString(
    std::string json_str, float scale,
    CompositionAssetLoader::CallbackType callback) {
  auto request = CompositionModelRequest{
      .json_str = std::move(json_str),
      .scale = scale,
      .enable_audio = enable_audio_,
  };
  LoadCompositionModel(std::move(request), std::move(callback));
}

void CompositionModelPipeline::LoadCompositionModelAsset(
    std::shared_ptr<CompositionModel> model,
    CompositionAssetLoader::CallbackType callback) {
  auto exec_callback = MakeCallback(std::move(callback));
  auto request = CompositionAssetRequest{
      .main_uri_info = main_uri_info_,
      .model = std::move(model),
  };

  if (!composition_asset_loader_) {
    exec_callback(
        {}, LoaderError{.code = LoaderErrorCode::kInvalidLoader,
                        .message = "Composition asset loader is not ready."});
    return;
  }

  composition_asset_loader_->Load(std::move(request), std::move(exec_callback));
}

void CompositionModelPipeline::SetSrcPolyfill(
    std::unordered_map<std::string, std::string> polyfill) {
  main_uri_info_.polyfill = std::move(polyfill);
}

void CompositionModelPipeline::SetImageFolder(std::string image_folder) {
  main_uri_info_.image_folder = std::move(image_folder);
}

void CompositionModelPipeline::SetHasDynamicResource(
    bool has_dynamic_resource) {
  has_dynamic_resource_ = has_dynamic_resource;
}

void CompositionModelPipeline::SetEnableAudio(bool enable) {
  enable_audio_ = enable;
}

void CompositionModelPipeline::OnResourceLoaderTraceEvent(
    TraceEventType trace_event, std::string main_uri, std::string asset_id) {
  Trace(trace_event);

  switch (trace_event) {
    case TraceEventType::kRequestCompositionStart:
      ANIMAX_TRACE_INSTANT_MAIN_URI(kRequestCompositionStart, main_uri);
      break;
    case TraceEventType::kParseCompositionStart:
      ANIMAX_TRACE_EVENT_BEGIN_MAIN_URI(kParseComposition, main_uri);
      break;
    case TraceEventType::kPrepareAssetsStart:
      ANIMAX_TRACE_INSTANT_MAIN_URI(kPrepareAssetsStart, main_uri);
      break;
    case TraceEventType::kPrepareSingleAssetStart:
      ANIMAX_TRACE_INSTANT_ASSET_ID(kPrepareSingleAssetStart, asset_id);
      break;
    case TraceEventType::kRequestCompositionEnd:
      ANIMAX_TRACE_INSTANT_MAIN_URI(kRequestCompositionEnd, main_uri);
      break;
    case TraceEventType::kParseCompositionEnd:
      ANIMAX_TRACE_EVENT_END();
      break;
    case TraceEventType::kPrepareAssetsEnd:
      ANIMAX_TRACE_INSTANT_MAIN_URI(kPrepareAssetsEnd, main_uri);
      break;
    case TraceEventType::kPrepareSingleAssetEnd:
      ANIMAX_TRACE_INSTANT_ASSET_ID(kPrepareSingleAssetEnd, asset_id);
      break;
    default:
      break;
  }
}

void CompositionModelPipeline::OnBeforeAssetsLoad(
    const std::vector<std::shared_ptr<Asset>>& assets,
    const std::vector<std::shared_ptr<Asset>>& invalid_assets) {
  for (const auto& invalid_asset : invalid_assets) {
    if (invalid_asset.get()) {
      ANIMAX_RESOURCE_LOGW(
          "Asset [ " << AssetVerboseWrapper(invalid_asset.get())
                     << " ] has an invalid URI or has already been loaded. "
                        "Skipping asset resource loading.");
    }
  }

  if (assets.empty()) {
    ANIMAX_RESOURCE_LOGI("No asset to load.");
    return;
  }

  ANIMAX_RESOURCE_LOGI("Will load the following " << assets.size());
}

void CompositionModelPipeline::Trace(TraceEventType type) {
  resource_thread_recorder_.Trace(type);
}

TimestampArray CompositionModelPipeline::ExportTimestamps() const {
  return resource_thread_recorder_.Export();
}

}  // namespace animax
}  // namespace lynx
