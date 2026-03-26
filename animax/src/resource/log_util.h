// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_RESOURCE_LOG_UTIL_H_
#define ANIMAX_SRC_RESOURCE_LOG_UTIL_H_

#include <ostream>

#include "src/base/log/log.h"

#define ANIMAX_RESOURCE_LOG(LEVEL, msg) \
  ANIMAX_LOG##LEVEL("[AnimaXResource]" << msg)

#define ANIMAX_RESOURCE_LOGV(msg) ANIMAX_RESOURCE_LOG(V, msg)
#define ANIMAX_RESOURCE_LOGD(msg) ANIMAX_RESOURCE_LOG(D, msg)
#define ANIMAX_RESOURCE_LOGI(msg) ANIMAX_RESOURCE_LOG(I, msg)
#define ANIMAX_RESOURCE_LOGW(msg) ANIMAX_RESOURCE_LOG(W, msg)
#define ANIMAX_RESOURCE_LOGE(msg) ANIMAX_RESOURCE_LOG(E, msg)
#define ANIMAX_RESOURCE_LOGR(msg) ANIMAX_RESOURCE_LOG(R, msg)
#define ANIMAX_RESOURCE_LOGF(msg) ANIMAX_RESOURCE_LOG(F, msg)

namespace lynx {
namespace animax {

class Asset;
struct LoaderError;
struct CompositionAssetResponse;
struct AssetResponse;

class AssetVerboseWrapper {
 public:
  explicit AssetVerboseWrapper(Asset* asset) : asset_(asset) {}
  friend std::ostream& operator<<(std::ostream& os,
                                  const AssetVerboseWrapper& wrapper);

 private:
  Asset* asset_;
};

std::ostream& operator<<(std::ostream& os, Asset& asset);
std::ostream& operator<<(std::ostream& os, const AssetVerboseWrapper& wrapper);
std::ostream& operator<<(std::ostream& os, const LoaderError& error);
std::ostream& operator<<(std::ostream& os, const AssetResponse& response);
std::ostream& operator<<(std::ostream& os, const CompositionAssetResponse& res);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_LOG_UTIL_H_
