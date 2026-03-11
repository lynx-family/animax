// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_JSON_COMPOSITION_MODEL_LOADER_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_JSON_COMPOSITION_MODEL_LOADER_H_

#include <memory>

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/resource/composition_loader/composition_model_loader.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/composition_loader/composition_task_stage.h"
#include "src/resource/resource_loader_listener.h"

namespace lynx {
namespace animax {

class JSONCompositionModelLoader : public CompositionModelLoader {
 public:
  static Ptr MakeLoader(
      Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
      std::weak_ptr<ResourceLoaderListener> listener = {});
  ~JSONCompositionModelLoader() override = default;
  void Load(CompositionModelRequest request, CallbackType callback) override;

 private:
  explicit JSONCompositionModelLoader(
      std::weak_ptr<ResourceLoaderListener> listener = {});
  static Ptr MakePipeline(
      Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
      std::weak_ptr<JSONCompositionModelLoader> weak_self);
  Ptr pipeline_{};
  std::weak_ptr<ResourceLoaderListener> listener_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_JSON_COMPOSITION_MODEL_LOADER_H_
