// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/json_composition_model_loader.h"

#include <memory>

#include "include/resource/loader_error.h"
#include "include/resource/raw_data.h"
#include "include/resource/resource_task.h"
#include "include/resource/uri_info.h"
#include "src/base/monitor/trace_event.h"
#include "src/parser/composition_parser.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/resource_loader_listener.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

namespace {

static constexpr const char* kJSONMainURIPlaceHolder = "[[JSON]]";

struct LottieCompositionModelTask {
  CompositionModelRequest request{};
  std::unique_ptr<RawData> json_data{};
  CompositionModelResponse response{};
  std::string main_uri{};
};

std::unique_ptr<RawData> MakeRawDataFromString(std::string str) {
  auto* str_ptr = new std::string{std::move(str)};
  auto raw_data = RawData::MakeRawData(
      str_ptr->data(), str_ptr->length(),
      [](const auto* str_ptr) {
        delete reinterpret_cast<const std::string*>(str_ptr);
      },
      str_ptr);
  return raw_data;
}
}  // namespace

JSONCompositionModelLoader::Ptr JSONCompositionModelLoader::MakePipeline(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    std::weak_ptr<JSONCompositionModelLoader> weak_self) {
  // Initializes a pipeline that consists of 2 distinct steps to process a
  // CompositionModelRequest into a CompositionModelResponse for Lottie
  // animations.

  // Step 1: Retrieves raw data based on the request parameters; directly from
  // JSON string if provided, otherwise via a URI.
  auto task_get_raw_data = MakeLambdaLoader<CompositionModelRequest,
                                            LottieCompositionModelTask>(
      [resource_loader, weak_self](CompositionModelRequest request,
                                   auto callback) {
        if (request.json_str.empty() && !request.uri_info.Valid()) {
          callback(
              {},
              LoaderError{
                  .code = kInvalidArgument,
                  .message =
                      "To load an Lottie composition, Either a valid URI or a "
                      "JSON string must be provided in CompositionRequest."});
          return;
        }

        auto base_uri = ParseUriMainResourceBaseUri(request.uri_info.uri);
        auto main_uri = request.uri_info.uri;
        auto task = LottieCompositionModelTask{
            .request = std::move(request),
            .response = CompositionModelResponse{
                .model_type = CompositionModelType::kLottie,
                .base_uri = std::move(base_uri),
                .main_uri = main_uri,
            }};
        if (!task.request.json_str.empty()) {
          // For JSON-data-based Compositions
          task.main_uri = kJSONMainURIPlaceHolder;
          task.response.main_uri = kJSONMainURIPlaceHolder;
          OnResourceLoaderTraceEvent(weak_self,
                                     TraceEventType::kRequestCompositionStart,
                                     task.main_uri);
          task.json_data =
              MakeRawDataFromString(std::move(task.request.json_str));
          OnResourceLoaderTraceEvent(
              weak_self, TraceEventType::kRequestCompositionEnd, task.main_uri);
          callback(std::move(task), {});
          return;
        }

        // URI-based Compositions
        task.main_uri = task.request.uri_info.uri;
        auto resource_request = ResourceRequest{
            .type = ResourceRequestType::kLoadRawData,
            .uri_info = std::move(task.request.uri_info),
        };
        OnResourceLoaderTraceEvent(
            weak_self, TraceEventType::kRequestCompositionStart, task.main_uri);
        resource_loader->Load(
            std::move(resource_request),
            [task = std::move(task), callback = std::move(callback), weak_self](
                ResourceResponse response, LoaderError error) mutable {
              OnResourceLoaderTraceEvent(weak_self,
                                         TraceEventType::kRequestCompositionEnd,
                                         task.main_uri);
              if (error) {
                callback({}, std::move(error));
                return;
              }

              task.json_data = std::move(response.payload.raw_data);
              callback(std::move(task), {});
            });
      });

  // Step 2: Parses the raw data into a Lottie model, handling errors in parsing
  // and providing the model to the task.
  auto raw_data_to_model =
      MakeLambdaLoader<LottieCompositionModelTask, CompositionModelResponse>(
          [weak_self](LottieCompositionModelTask task, auto callback) mutable {
            auto raw_data = std::move(task.json_data);
            OnResourceLoaderTraceEvent(weak_self,
                                       TraceEventType::kParseCompositionStart,
                                       task.main_uri);
            auto model = CompositionParser::Parse(
                static_cast<const char*>(raw_data->Data()), raw_data->Length(),
                task.request.scale);
            OnResourceLoaderTraceEvent(
                weak_self, TraceEventType::kParseCompositionEnd, task.main_uri);
            if (!model) {
              callback({}, LoaderError{.code = kCompositionParserError});
              return;
            }

            task.response.model = std::move(model);
            if (!task.response.model) {
              callback(
                  std::move(task.response),
                  LoaderError{.code = kCompositionParserError,
                              "Failed to convert JSON into CompositionModel."});
            } else {
              callback(std::move(task.response), {});
            }
          });

  auto pipeline = task_get_raw_data | raw_data_to_model;
  return pipeline;
}

JSONCompositionModelLoader::Ptr JSONCompositionModelLoader::MakeLoader(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    std::weak_ptr<ResourceLoaderListener> listener) {
  auto loader = std::shared_ptr<JSONCompositionModelLoader>(
      new JSONCompositionModelLoader{std::move(listener)});
  loader->pipeline_ = MakePipeline(std::move(resource_loader), loader);
  return loader;
}

JSONCompositionModelLoader::JSONCompositionModelLoader(
    std::weak_ptr<ResourceLoaderListener> listener)
    : CompositionModelLoader(std::move(listener)) {}

void JSONCompositionModelLoader::Load(CompositionModelRequest request,
                                      CallbackType callback) {
  pipeline_->Load(std::move(request), std::move(callback));
}

}  // namespace animax
}  // namespace lynx
