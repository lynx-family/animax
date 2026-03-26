// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/json_alpha_video_model_loader.h"

#include <memory>

#include "include/resource/loader_error.h"
#include "include/resource/resource_task.h"
#include "include/resource/uri_info.h"
#include "src/base/monitor/trace_event.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/composition_loader/zip_alpha_video_model_loader.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/resource_loader_listener.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

JSONAlphaVideoModelLoader::Ptr JSONAlphaVideoModelLoader::MakePipeline(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    std::weak_ptr<JSONAlphaVideoModelLoader> weak_model_loader) {
  // Step 1: convert the CompositionModelRequest to a ZipCompositionModelTask
  // if the request is a local file alpha video json file.
  auto convert_to_zip_task =
      MakeLambdaLoader<CompositionModelRequest, ZipCompositionModelTask>(
          [weak_model_loader](CompositionModelRequest request,
                              auto callback) mutable {
            if (request.uri_info.scheme != UriInfo::Scheme::kFile) {
              callback(ZipCompositionModelTask{},
                       LoaderError{
                           .code = kInvalidArgument,
                           .message =
                               "To load a local file alpha video composition, "
                               "a file scheme URI must be provided in "
                               "CompositionRequest."});
              return;
            }

            const auto main_uri = request.uri_info.uri;
            const auto main_uri_path = GetUriLocalPath(main_uri);
            CompositionModelLoader::OnResourceLoaderTraceEvent(
                weak_model_loader, TraceEventType::kRequestCompositionStart,
                main_uri);

            auto task = ZipCompositionModelTask{
                .request = std::move(request),
                .response =
                    {
                        .model_type = CompositionModelType::kAlphaVideo,
                        .base_uri = main_uri_path,
                        .main_uri = main_uri_path,
                    },
            };
            CompositionModelLoader::OnResourceLoaderTraceEvent(
                weak_model_loader, TraceEventType::kRequestCompositionEnd,
                main_uri);
            callback(std::move(task), {});
          });

  // Step 2: load the alpha video parent directory as a unzipped alpha video
  // model.
  auto zip_alpha_video_loader =
      ZipAlphaVideoModelLoader::MakeLoader(resource_loader, weak_model_loader);

  auto pipeline = convert_to_zip_task | zip_alpha_video_loader;
  return pipeline;
}

JSONAlphaVideoModelLoader::Ptr JSONAlphaVideoModelLoader::MakeLoader(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    std::weak_ptr<ResourceLoaderListener> listener) {
  auto loader = std::shared_ptr<JSONAlphaVideoModelLoader>(
      new JSONAlphaVideoModelLoader{std::move(listener)});
  loader->pipeline_ = MakePipeline(std::move(resource_loader), loader);
  return loader;
}

JSONAlphaVideoModelLoader::JSONAlphaVideoModelLoader(
    std::weak_ptr<ResourceLoaderListener> listener)
    : CompositionModelLoader(std::move(listener)) {}

void JSONAlphaVideoModelLoader::Load(CompositionModelRequest request,
                                     CallbackType callback) {
  pipeline_->Load(std::move(request), std::move(callback));
}

}  // namespace animax
}  // namespace lynx
