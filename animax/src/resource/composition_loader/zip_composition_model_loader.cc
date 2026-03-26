// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/zip_composition_model_loader.h"

#include <sys/stat.h>

#include <memory>

#include "src/resource/composition_loader/zip_alpha_video_model_loader.h"
#include "src/resource/composition_loader/zip_composition_unzip_loader.h"
#include "src/resource/composition_loader/zip_json_model_loader.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

void ZipCompositionModelLoader::Load(CompositionModelRequest request,
                                     CallbackType callback) {
  download_unzip_loader_->Load(
      std::move(request),
      [callback = std::move(callback), json_loader = zip_json_loader_,
       alpha_video_loader = zip_alpha_video_loader_](
          ZipCompositionModelTask task, LoaderError error) mutable {
        if (error) {
          callback(CompositionModelResponse{}, std::move(error));
          return;
        }
        if (task.response.model_type == CompositionModelType::kLottie) {
          json_loader->Load(std::move(task), std::move(callback));
        } else {
          alpha_video_loader->Load(std::move(task), std::move(callback));
        }
      });
}

ZipCompositionModelLoader::Ptr ZipCompositionModelLoader::MakeLoader(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader,
    std::weak_ptr<ResourceLoaderListener> listener) {
  auto loader = std::shared_ptr<ZipCompositionModelLoader>(
      new ZipCompositionModelLoader{std::move(listener)});
  auto weak_self = loader->weak_from_this();
  loader->download_unzip_loader_ = ZipCompositionUnzipLoader::MakeLoader(
      resource_loader, unzip_loader, weak_self);
  loader->zip_alpha_video_loader_ =
      ZipAlphaVideoModelLoader::MakeLoader(resource_loader, weak_self);
  loader->zip_json_loader_ =
      ZipJSONModelLoader::MakeLoader(resource_loader, weak_self);
  return loader;
}

ZipCompositionModelLoader::ZipCompositionModelLoader(
    std::weak_ptr<ResourceLoaderListener> listener)
    : CompositionModelLoader(std::move(listener)) {}

}  // namespace animax
}  // namespace lynx
