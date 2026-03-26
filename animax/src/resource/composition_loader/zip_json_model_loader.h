// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_JSON_MODEL_LOADER_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_JSON_MODEL_LOADER_H_

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/resource/composition_loader/composition_model_loader.h"
#include "src/resource/composition_loader/composition_model_pipeline.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/composition_loader/zip_composition_task.h"

namespace lynx {
namespace animax {

class ZipJSONModelLoader
    : public Loader<ZipCompositionModelTask, CompositionModelResponse> {
 public:
  ~ZipJSONModelLoader() override = default;
  void Load(ZipCompositionModelTask task, CallbackType callback) override;
  static ZipJSONModelLoader::Ptr MakeLoader(
      Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
      std::weak_ptr<ModelLoader> composition_loader = {});

 private:
  explicit ZipJSONModelLoader(
      std::weak_ptr<ModelLoader> composition_loader = {});
  static Ptr MakePipeline(
      Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
      std::weak_ptr<ModelLoader> weak_model_loader);
  Ptr pipeline_{};
  std::weak_ptr<ModelLoader> weak_model_loader_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_JSON_MODEL_LOADER_H_
