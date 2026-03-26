// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_LISTENER_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_LISTENER_H_

#include <memory>
#include <string>
#include <vector>

#include "src/base/monitor/trace_event.h"

namespace lynx {
namespace animax {

class Asset;
class ResourceLoaderListener {
 public:
  virtual ~ResourceLoaderListener() = default;
  virtual void OnResourceLoaderTraceEvent(TraceEventType trace_event,
                                          std::string arg1 = {},
                                          std::string arg2 = {}){};
  virtual void OnBeforeAssetsLoad(
      const std::vector<std::shared_ptr<Asset>>& assets,
      const std::vector<std::shared_ptr<Asset>>& invalid_assets){};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_LISTENER_H_
