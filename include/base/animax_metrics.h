// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_BASE_ANIMAX_METRICS_H_
#define ANIMAX_INCLUDE_BASE_ANIMAX_METRICS_H_

#include <string>
#include <unordered_map>

#include "base/include/closure.h"

namespace lynx {
namespace animax {

using MetricsMap = std::unordered_map<std::string, double>;

using ExternalMetricsReadyCallback = base::MoveOnlyClosure<void, MetricsMap>;

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_BASE_ANIMAX_METRICS_H_
