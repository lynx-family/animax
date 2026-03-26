// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_H_

#include <memory>
#include <string>

#include "base/include/closure.h"
#include "include/resource/bitmap.h"
#include "include/resource/raw_data.h"

namespace lynx {
namespace animax {

std::string GetCacheDirectory();

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_H_
