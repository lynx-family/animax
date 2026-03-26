// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader.h"
#include "src/resource/resource_loader/ios/resource_util.h"

namespace lynx {
namespace animax {

std::string GetCacheDirectory() { return EnsureCacheDirectory(); }

}  // namespace animax
}  // namespace lynx
