// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/composition_model_loader.h"

namespace lynx {
namespace animax {

void CompositionModelLoader::OnResourceLoaderTraceEvent(
    std::weak_ptr<Loader<CompositionModelRequest, CompositionModelResponse>>
        weak_self,
    TraceEventType event, std::string main_uri, std::string asset_id) {
  auto self = weak_self.lock();
  if (!self) {
    return;
  }
  auto listener =
      std::static_pointer_cast<CompositionModelLoader>(self)->listener_.lock();
  if (!listener) {
    return;
  }
  listener->OnResourceLoaderTraceEvent(event, std::move(main_uri),
                                       std::move(asset_id));
}

}  // namespace animax
}  // namespace lynx
