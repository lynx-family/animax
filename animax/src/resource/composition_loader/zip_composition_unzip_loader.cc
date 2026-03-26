// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/zip_composition_unzip_loader.h"

#include <sys/stat.h>

#include "include/resource/loader_error.h"
#include "include/resource/uri_info.h"
#include "src/base/monitor/trace_event.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

namespace {

static constexpr const char* kConfigJSONFileName = "config.json";

}  // namespace

ZipCompositionUnzipLoader::Ptr ZipCompositionUnzipLoader::MakePipeline(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader,
    std::weak_ptr<ModelLoader> weak_model_loader) {
  // Initializes a pipeline that consists of 2 distinct steps to process a
  // CompositionModelRequest into a ZipCompositionModelTask.

  // Step 1: Downloads the .zip file specified in the task using a resource
  // loader, updating the task with the download path.
  auto download_zip =
      MakeLambdaLoader<CompositionModelRequest, ZipCompositionModelTask>(
          [resource_loader, weak_model_loader](CompositionModelRequest request,
                                               auto callback) mutable {
            if (!request.uri_info.Valid() ||
                request.uri_info.content_type != UriInfo::ContentType::kZip) {
              callback(ZipCompositionModelTask{},
                       LoaderError{
                           .code = kInvalidArgument,
                           .message =
                               "To load a zip composition, a valid .zip "
                               "URI must be provided in CompositionRequest."});
              return;
            }

            const auto main_uri = request.uri_info.uri;
            // For ZipCompositionModel, request starts before fetching
            // the zip file and ends after the zip file is unzipped in Step 2.
            CompositionModelLoader::OnResourceLoaderTraceEvent(
                weak_model_loader, TraceEventType::kRequestCompositionStart,
                main_uri);

            auto task = ZipCompositionModelTask{.request = std::move(request)};
            ResourceRequest resource_request{
                .type = ResourceRequestType::kDownloadToLocal,
                .uri_info = task.request.uri_info,
            };
            resource_loader->Load(
                std::move(resource_request),
                [task = std::move(task), callback = std::move(callback),
                 weak_model_loader,
                 main_uri](ResourceResponse response, auto error) mutable {
                  if (error) {
                    callback({}, error);
                    CompositionModelLoader::OnResourceLoaderTraceEvent(
                        weak_model_loader,
                        TraceEventType::kRequestCompositionEnd, main_uri);
                    return;
                  }
                  auto zip_file_path =
                      UriInfo{.scheme = ParseUriScheme(response.payload.path),
                              .content_type = UriInfo::ContentType::kZip,

                              .uri = response.payload.path};
                  task.zip_file_path = std::move(zip_file_path);
                  callback(std::move(task), {});
                });
          });
  // Step 2: Unzips the downloaded .zip file, updating the
  // task with the path to the unzipped folder.
  // TODO(sonny.fu): Can add an extra file magic number check to early fail if
  // not a valid zip file.
  auto unzip =
      MakeLambdaLoader<ZipCompositionModelTask, ZipCompositionModelTask>(
          [unzip_loader, weak_model_loader](ZipCompositionModelTask task,
                                            auto callback) mutable {
            UnzipRequest unzip_request{
                .zip_file_path = task.zip_file_path.uri,
            };

            unzip_loader->Load(
                std::move(unzip_request),
                [task = std::move(task), callback = std::move(callback),
                 weak_model_loader](UnzipResponse response,
                                    auto error) mutable {
                  // ZipCompositionModel request stage ends here.
                  CompositionModelLoader::OnResourceLoaderTraceEvent(
                      weak_model_loader, TraceEventType::kRequestCompositionEnd,
                      task.request.uri_info.uri);
                  if (error) {
                    callback(ZipCompositionModelTask{}, error);
                    return;
                  }
                  auto unzip_folder_path = response.path;

                  // The base URI for this CompositionModel is the unzipped
                  // folder.
                  task.response.base_uri = unzip_folder_path;

                  // we consider every alpha video zip contains "config.json"
                  auto config_json_file_path =
                      ConcatFilePaths(unzip_folder_path, kConfigJSONFileName);
                  task.response.main_uri = config_json_file_path;
                  if (IsRegularFileExists(config_json_file_path)) {
                    task.response.model_type =
                        CompositionModelType::kAlphaVideo;
                  } else {
                    task.response.model_type = CompositionModelType::kLottie;
                  }

                  callback(std::move(task), {});
                });
          });

  auto pipeline = download_zip | unzip;
  return pipeline;
}

ZipCompositionUnzipLoader::Ptr ZipCompositionUnzipLoader::MakeLoader(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader,
    std::weak_ptr<ModelLoader> composition_loader) {
  auto loader = std::shared_ptr<ZipCompositionUnzipLoader>(
      new ZipCompositionUnzipLoader{std::move(composition_loader)});
  loader->pipeline_ =
      MakePipeline(std::move(resource_loader), std::move(unzip_loader),
                   loader->weak_model_loader_);
  return loader;
}

ZipCompositionUnzipLoader::ZipCompositionUnzipLoader(
    std::weak_ptr<ModelLoader> composition_loader)
    : weak_model_loader_(composition_loader) {}

void ZipCompositionUnzipLoader::Load(CompositionModelRequest request,
                                     Loader::CallbackType callback) {
  pipeline_->Load(std::move(request), std::move(callback));
}

}  // namespace animax
}  // namespace lynx
