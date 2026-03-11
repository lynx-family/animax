// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_COMPOSITION_MODEL_LOADER_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_COMPOSITION_MODEL_LOADER_H_

#include <memory>

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "include/resource/unzip_task.h"
#include "src/resource/composition_loader/composition_model_loader.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/composition_loader/zip_composition_task.h"
#include "src/resource/resource_loader_listener.h"

namespace lynx {
namespace animax {

class ZipCompositionModelLoader : public CompositionModelLoader {
 public:
  ~ZipCompositionModelLoader() override = default;
  void Load(CompositionModelRequest request, CallbackType callback) override;
  static ZipCompositionModelLoader::Ptr MakeLoader(
      Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
      Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader,
      std::weak_ptr<ResourceLoaderListener> listener = {});

 private:
  explicit ZipCompositionModelLoader(
      std::weak_ptr<ResourceLoaderListener> listener = {});

  Loader<CompositionModelRequest, ZipCompositionModelTask>::Ptr
      download_unzip_loader_;
  Loader<ZipCompositionModelTask, CompositionModelResponse>::Ptr
      zip_json_loader_;
  Loader<ZipCompositionModelTask, CompositionModelResponse>::Ptr
      zip_alpha_video_loader_;

  std::weak_ptr<ResourceLoaderListener> listener_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_COMPOSITION_MODEL_LOADER_H_
