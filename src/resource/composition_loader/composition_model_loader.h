// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_MODEL_LOADER_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_MODEL_LOADER_H_

#include "include/resource/loader.h"
#include "src/base/monitor/trace_event.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/resource_loader_listener.h"
namespace lynx {
namespace animax {

class CompositionModelLoader
    : public Loader<CompositionModelRequest, CompositionModelResponse> {
 public:
  explicit CompositionModelLoader(
      std::weak_ptr<ResourceLoaderListener> listener)
      : listener_(std::move(listener)) {}
  ~CompositionModelLoader() override = default;

  static void OnResourceLoaderTraceEvent(
      std::weak_ptr<Loader<CompositionModelRequest, CompositionModelResponse>>
          weak_self,
      TraceEventType event, std::string main_uri = {},
      std::string asset_id = {});

 private:
  std::weak_ptr<ResourceLoaderListener> listener_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_MODEL_LOADER_H_
