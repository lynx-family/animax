// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/composition_asset_loader.h"

#include "include/resource/loader_error.h"
#include "include/resource/resource_task.h"
#include "include/resource/uri_info.h"
#include "src/base/log/log.h"
#include "src/base/monitor/trace_event.h"
#include "src/model/composition_model.h"
#include "src/resource/asset/asset.h"
#include "src/resource/asset/audio_asset.h"
#include "src/resource/asset/font_asset.h"
#include "src/resource/asset/image_asset.h"
#include "src/resource/asset/video_asset.h"
#include "src/resource/loader/all_settled_loader.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

namespace {

class ResourceRequestBuilder : public AssetVisitor {
 public:
  explicit ResourceRequestBuilder(MainResourceUriInfo* main_uri_info)
      : main_uri_info_(main_uri_info) {}

  ~ResourceRequestBuilder() override = default;
  void Visit(FontAsset& asset) override {
    request_ = ResourceRequest{
        .type = ResourceRequestType::kLoadRawData,
        .uri_info = UriInfoFromFontAsset(*main_uri_info_, asset),
        .id = asset.Model().name,
        .main_uri = main_uri_info_->main_uri,
    };
  }
  void Visit(ImageAsset& asset) override {
    request_ = ResourceRequest{
        .type = ResourceRequestType::kLoadBitmap,
        .uri_info = UriInfoFromImageAsset(*main_uri_info_, asset),
        .width = asset.Model().width,
        .height = asset.Model().height,
        .id = asset.Model().id,
        .main_uri = main_uri_info_->main_uri,
    };
  }
  void Visit(VideoAsset& asset) override {
    request_ = ResourceRequest{
        .type = ResourceRequestType::kDownloadToLocal,
        .uri_info = UriInfoFromVideoAsset(*main_uri_info_, asset),
        .id = asset.Model().id,
        .main_uri = main_uri_info_->main_uri,
    };
  }
  void Visit(AudioAsset& asset) override {
    request_ = ResourceRequest{
        .type = ResourceRequestType::kDownloadToLocal,
        .uri_info = UriInfoFromAudioAsset(*main_uri_info_, asset),
        .id = asset.Model().id,
        .main_uri = main_uri_info_->main_uri,
    };
  }

  ResourceRequest ConsumeResourceRequest() { return std::move(request_); }

 private:
  MainResourceUriInfo* main_uri_info_;
  ResourceRequest request_;
};

class ResourceResponseConsumer : public AssetVisitor {
 public:
  ~ResourceResponseConsumer() override = default;

  void SetResourceLoaderResult(ResourceResponse resource_response,
                               LoaderError error) {
    resource_response_ = std::move(resource_response);
    error_ = std::move(error);
  }

  AssetResponse ConsumeAssetResponse() { return std::move(asset_response_); }

  void Visit(FontAsset& asset) override {
    if (!error_) {
      asset.SetRawData(std::move(resource_response_.payload.raw_data));
    }
    asset_response_ = {.type = ResourceType::kFont,
                       .id = asset.Model().family + asset.Model().name,
                       .error = std::move(error_)};
  }

  void Visit(ImageAsset& asset) override {
    if (!error_) {
      asset.SetBitmap(std::move(resource_response_.payload.bitmap));
    }
    asset_response_ = {
        .type = ResourceType::kImage,
        .id = asset.Model().id,
        .error = std::move(error_),
    };
  }

  void Visit(VideoAsset& asset) override {
    if (!error_) {
      asset.PrepareFrameData(resource_response_.payload.path);
    }
    asset_response_ = {
        .type = ResourceType::kVideo,
        .id = asset.Model().id,
        .error = std::move(error_),
    };
  }

  void Visit(AudioAsset& asset) override {
    if (!error_) {
      asset.SetLocalPath(resource_response_.payload.path);
    }
    asset_response_ = {
        .type = ResourceType::kAudio,
        .id = asset.Model().id,
        .error = std::move(error_),
    };
  }

 private:
  ResourceResponse resource_response_{};
  LoaderError error_{};
  AssetResponse asset_response_{};
};

std::vector<std::shared_ptr<Asset>> MergeCompositionModelAssets(
    const std::shared_ptr<CompositionModel>& model) {
  const auto size = model->GetFonts().size() + model->GetImages().size() +
                    model->GetVideos().size();
  auto assets = std::vector<std::shared_ptr<Asset>>{};
  assets.reserve(size);
  for (auto& [id, asset] : model->GetFonts()) {
    assets.push_back(asset);
  }
  for (auto& [id, asset] : model->GetImages()) {
    assets.push_back(asset);
  }
  for (auto& [id, asset] : model->GetVideos()) {
    assets.push_back(asset);
  }
  for (auto& [id, asset] : model->GetAudios()) {
    assets.push_back(asset);
  }
  return assets;
}

}  // namespace

CompositionAssetLoader::CompositionAssetLoader(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    std::weak_ptr<ResourceLoaderListener> listener)
    : listener_{listener} {
  // Make a special Loader that will trace when the asset starts or finishes
  // loading.
  auto listened_loader = MakeLambdaLoader<ResourceRequest, ResourceResponse>(
      [resource_loader = std::move(resource_loader), weak_listener = listener](
          ResourceRequest request,
          Loader<ResourceRequest, ResourceResponse>::CallbackType callback) {
        if (auto listener = weak_listener.lock(); listener) {
          listener->OnResourceLoaderTraceEvent(
              TraceEventType::kPrepareSingleAssetStart, request.main_uri,
              request.id);
        }
        resource_loader->Load(
            request, [weak_listener, main_uri = request.main_uri,
                      id = request.id, callback = std::move(callback)](
                         ResourceResponse response, LoaderError error) {
              if (auto listener = weak_listener.lock(); listener) {
                listener->OnResourceLoaderTraceEvent(
                    TraceEventType::kPrepareSingleAssetEnd, main_uri, id);
              }
              callback(std::move(response), std::move(error));
            });
      });
  resource_loader_ = AllSettledLoader<ResourceRequest, ResourceResponse>::Make<
      AllSettledLoader<ResourceRequest, ResourceResponse>>(
      std::move(listened_loader));
}

void CompositionAssetLoader::Load(CompositionAssetRequest request,
                                  CallbackType callback) {
  auto assets = MergeCompositionModelAssets(request.model);
  auto resource_request_build = ResourceRequestBuilder{&request.main_uri_info};
  auto resource_requests = std::vector<ResourceRequest>{};
  resource_requests.reserve(assets.size());
  auto need_load_assets = std::vector<std::shared_ptr<Asset>>{};
  auto invalid_assets = std::vector<std::shared_ptr<Asset>>{};

  for (const auto& asset : assets) {
    if (!asset) {
      continue;
    }
    asset->AcceptVisitor(resource_request_build);
    auto resource_request = resource_request_build.ConsumeResourceRequest();
    if (resource_request.uri_info.Valid()) {
      resource_requests.emplace_back(std::move(resource_request));
      need_load_assets.push_back(asset);
    } else {
      invalid_assets.push_back(asset);
    }
  }

  const auto total_need_load_asset_count = need_load_assets.size();
  DCHECK(total_need_load_asset_count == need_load_assets.size());
  DCHECK(total_need_load_asset_count == resource_requests.size());

  if (total_need_load_asset_count == 0) {
    callback(CompositionAssetResponse{.model = std::move(request.model)}, {});
    return;
  }

  // We have some assets to load.
  const auto main_uri = request.main_uri_info.main_uri;
  if (auto listener = listener_.lock(); listener) {
    listener->OnResourceLoaderTraceEvent(TraceEventType::kPrepareAssetsStart,
                                         main_uri);
    listener->OnBeforeAssetsLoad(need_load_assets, invalid_assets);
  }

  resource_loader_->Load(
      std::move(resource_requests),
      [total_need_load_asset_count,
       need_load_assets = std::move(need_load_assets), model = request.model,
       callback = std::move(callback), weak_listener = listener_,
       main_uri](AllSettledResponse<ResourceResponse> responses,
                 LoaderError error) mutable {
        auto resource_response_consumer = ResourceResponseConsumer{};
        auto asset_responses = std::vector<AssetResponse>{};
        asset_responses.reserve(total_need_load_asset_count);
        DCHECK(total_need_load_asset_count == need_load_assets.size());
        DCHECK(total_need_load_asset_count == responses.outs.size());
        DCHECK(total_need_load_asset_count == responses.errors.size());
        for (std::size_t i = 0; i < total_need_load_asset_count; ++i) {
          resource_response_consumer.SetResourceLoaderResult(
              std::move(responses.outs[i]), std::move(responses.errors[i]));
          need_load_assets[i]->AcceptVisitor(resource_response_consumer);
          asset_responses.push_back(
              resource_response_consumer.ConsumeAssetResponse());
        }
        if (auto listener = weak_listener.lock(); listener) {
          listener->OnResourceLoaderTraceEvent(
              TraceEventType::kPrepareAssetsEnd, main_uri);
        }
        model->SetAssetsLoaded(true);
        callback(CompositionAssetResponse{.model = std::move(model),
                                          .asset_responses =
                                              std::move(asset_responses)},
                 {});
      });
}

}  // namespace animax
}  // namespace lynx
