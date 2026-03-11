// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/zip_alpha_video_model_loader.h"

#include "include/resource/loader_error.h"
#include "src/base/monitor/trace_event.h"
#include "src/parser/alpha_player_asset/alpha_player_asset_parser.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

namespace {

static constexpr const char* kConfigJSONFileName = "config.json";

}  // namespace

ZipAlphaVideoModelLoader::Ptr ZipAlphaVideoModelLoader::MakePipeline(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    std::weak_ptr<ModelLoader> weak_model_loader) {
  // Initializes a pipeline that consists of 2 distinct steps to process a
  // ZipCompositionModelTask into a CompositionModelResponse.

  // Step 1: Loads and converts the alpha video configuration JSON file from the
  // unzipped folder into raw data.
  auto config_json_to_raw_data =
      MakeLambdaLoader<ZipCompositionModelTask, ZipCompositionModelTask>(
          [resource_loader](ZipCompositionModelTask task,
                            auto callback) mutable {
            auto unzip_folder_path = task.response.base_uri;
            auto config_json_file_path_str =
                ConcatFilePaths(unzip_folder_path, kConfigJSONFileName);
            auto config_json_file_path =
                UriInfo{.scheme = ParseUriScheme(config_json_file_path_str),
                        .content_type = UriInfo::ContentType::kJson,
                        .uri = config_json_file_path_str};
            task.json_file_path = std::move(config_json_file_path);

            ResourceRequest resource_request{
                .type = ResourceRequestType::kLoadRawData,
                .uri_info = task.json_file_path,
                .main_uri = config_json_file_path_str};
            resource_loader->Load(
                std::move(resource_request),
                [task = std::move(task), callback = std::move(callback)](
                    ResourceResponse response, auto error) mutable {
                  if (error) {
                    callback(ZipCompositionModelTask{}, error);
                    return;
                  }
                  task.json_data = std::move(response.payload.raw_data);
                  callback(std::move(task), {});
                });
          });

  // Step 2: Parses the raw data into a Lottie model, handling errors in parsing
  // and providing the model to the task.
  auto raw_data_to_model =
      MakeLambdaLoader<ZipCompositionModelTask, CompositionModelResponse>(
          [weak_model_loader](ZipCompositionModelTask task,
                              auto callback) mutable {
            auto raw_data = std::move(task.json_data);
            const auto unzip_folder_path = task.response.base_uri;
            const auto config_json_file_path_str =
                ConcatFilePaths(unzip_folder_path, kConfigJSONFileName);
            CompositionModelLoader::OnResourceLoaderTraceEvent(
                weak_model_loader, TraceEventType::kParseCompositionStart,
                config_json_file_path_str);
            auto model = AlphaPlayerAssetParser::Parse(
                task.response.base_uri,
                static_cast<const char*>(raw_data->Data()), raw_data->Length(),
                task.request.scale);
            CompositionModelLoader::OnResourceLoaderTraceEvent(
                weak_model_loader, TraceEventType::kParseCompositionEnd,
                config_json_file_path_str);

            if (!model) {
              callback({}, LoaderError{.code = kCompositionParserError});
              return;
            }

            task.response.model = std::move(model);

            if (!task.response.model || task.response.base_uri.empty()) {
              callback(
                  std::move(task.response),
                  LoaderError{
                      .code = kCompositionParserError,
                      "Failed to convert config.json into CompositionModel."});
            } else {
              callback(std::move(task.response), {});
            }
          });

  // Constructs the complete pipeline from the provided loaders.
  auto pipeline = config_json_to_raw_data | raw_data_to_model;
  return pipeline;
}

ZipAlphaVideoModelLoader::Ptr ZipAlphaVideoModelLoader::MakeLoader(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    std::weak_ptr<ModelLoader> composition_loader) {
  auto loader = std::shared_ptr<ZipAlphaVideoModelLoader>(
      new ZipAlphaVideoModelLoader{std::move(composition_loader)});
  loader->pipeline_ =
      MakePipeline(std::move(resource_loader), loader->weak_model_loader_);
  return loader;
}

ZipAlphaVideoModelLoader::ZipAlphaVideoModelLoader(
    std::weak_ptr<ModelLoader> composition_loader)
    : weak_model_loader_(composition_loader) {}

void ZipAlphaVideoModelLoader::Load(ZipCompositionModelTask task,
                                    Loader::CallbackType callback) {
  pipeline_->Load(std::move(task), std::move(callback));
}

}  // namespace animax
}  // namespace lynx
