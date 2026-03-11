// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_COMPOSITION_UNZIP_LOADER_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_COMPOSITION_UNZIP_LOADER_H_

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "include/resource/unzip_task.h"
#include "src/resource/composition_loader/composition_model_loader.h"
#include "src/resource/composition_loader/composition_model_pipeline.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/composition_loader/zip_composition_task.h"

namespace lynx {
namespace animax {

class ZipCompositionUnzipLoader
    : public Loader<CompositionModelRequest, ZipCompositionModelTask> {
 public:
  ~ZipCompositionUnzipLoader() override = default;
  void Load(CompositionModelRequest request, CallbackType callback) override;
  static ZipCompositionUnzipLoader::Ptr MakeLoader(
      Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
      Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader,
      std::weak_ptr<ModelLoader> composition_loader = {});

 private:
  explicit ZipCompositionUnzipLoader(
      std::weak_ptr<ModelLoader> composition_loader = {});
  static Ptr MakePipeline(
      Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
      Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader,
      std::weak_ptr<ModelLoader> weak_model_loader);
  Ptr pipeline_{};
  std::weak_ptr<ModelLoader> weak_model_loader_{};
};

}  // namespace animax

}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_COMPOSITION_UNZIP_LOADER_H_
