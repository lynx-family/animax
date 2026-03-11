// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/zip_json_model_loader.h"

#include "include/resource/loader_error.h"
#include "src/base/monitor/trace_event.h"
#include "src/parser/composition_parser.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

namespace {

static constexpr const char* kJSON = ".json";

}  // namespace

ZipJSONModelLoader::Ptr ZipJSONModelLoader::MakePipeline(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    std::weak_ptr<ModelLoader> weak_model_loader) {
  // Initializes a pipeline that consists of 2 distinct steps to process a
  // ZipCompositionModelTask into a CompositionModelResponse.

  // Step 1: Loads and converts the lottie data JSON file from the unzipped
  // folder into raw data.
  auto data_json_to_raw_data = MakeLambdaLoader<
      ZipCompositionModelTask,
      ZipCompositionModelTask>([resource_loader](ZipCompositionModelTask task,
                                                 auto callback) mutable {
    auto unzip_folder_path = task.response.base_uri;
    auto data_json_file_path_str =
        FindFirstMatchingFile(unzip_folder_path, kJSON);
    if (data_json_file_path_str.empty()) {
      callback(
          std::move(task),
          LoaderError{
              .code = kFileDoesNotExist,
              .message =
                  "No .json file found at the top level of the unzipped "
                  "contents. Please verify the structure of your zip file."});
      return;
    }
    auto data_json_file_path =
        UriInfo{.scheme = ParseUriScheme(data_json_file_path_str),
                .content_type = UriInfo::ContentType::kJson,
                .uri = data_json_file_path_str};
    task.json_file_path = std::move(data_json_file_path);

    ResourceRequest resource_request{
        .type = ResourceRequestType::kLoadRawData,
        .uri_info = task.json_file_path,
    };
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

  // Step 2: Parses the raw JSON data into a usable composition model.
  auto raw_data_to_model =
      MakeLambdaLoader<ZipCompositionModelTask, CompositionModelResponse>(
          [weak_model_loader](ZipCompositionModelTask task,
                              auto callback) mutable {
            auto raw_data = std::move(task.json_data);
            const auto main_uri = task.request.uri_info.uri;
            CompositionModelLoader::OnResourceLoaderTraceEvent(
                weak_model_loader, TraceEventType::kParseCompositionStart,
                main_uri);
            auto model = CompositionParser::Parse(
                static_cast<const char*>(raw_data->Data()), raw_data->Length(),
                task.request.scale);
            CompositionModelLoader::OnResourceLoaderTraceEvent(
                weak_model_loader, TraceEventType::kParseCompositionEnd,
                main_uri);

            if (!model) {
              callback({}, LoaderError{.code = kCompositionParserError});
              return;
            }

            task.response.model = std::move(model);

            if (!task.response.model || task.response.base_uri.empty()) {
              callback(
                  std::move(task.response),
                  LoaderError{.code = kCompositionParserError,
                              "Failed to convert json into CompositionModel."});
            } else {
              callback(std::move(task.response), {});
            }
          });

  // Constructs the complete pipeline from the provided loaders.
  auto pipeline = data_json_to_raw_data | raw_data_to_model;
  return pipeline;
}

ZipJSONModelLoader::Ptr ZipJSONModelLoader::MakeLoader(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    std::weak_ptr<ModelLoader> composition_loader) {
  auto loader = std::shared_ptr<ZipJSONModelLoader>(
      new ZipJSONModelLoader{std::move(composition_loader)});
  loader->pipeline_ =
      MakePipeline(std::move(resource_loader), loader->weak_model_loader_);
  return loader;
}

ZipJSONModelLoader::ZipJSONModelLoader(
    std::weak_ptr<ModelLoader> composition_loader)
    : weak_model_loader_(composition_loader) {}

void ZipJSONModelLoader::Load(ZipCompositionModelTask task,
                              Loader::CallbackType callback) {
  pipeline_->Load(std::move(task), std::move(callback));
}

}  // namespace animax
}  // namespace lynx
