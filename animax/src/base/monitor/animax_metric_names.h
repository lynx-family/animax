// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_MONITOR_ANIMAX_METRIC_NAMES_H_
#define ANIMAX_SRC_BASE_MONITOR_ANIMAX_METRIC_NAMES_H_

namespace lynx {
namespace animax {

class AnimaXMetricNames {
 public:
  static constexpr const char* kAnimaXFCP = "AnimaXFCP";
  static constexpr const char* kPrepareResourceTime = "PrepareResourceTime";
  static constexpr const char* kPrepareCompositionTime =
      "PrepareCompositionTime";
  static constexpr const char* kPrepareAssetsTime = "PrepareAssetsTime";
  static constexpr const char* kFirstFrameTime = "FirstFrameTime";
  static constexpr const char* kMeanFrameTime = "MeanFrameTime";
  static constexpr const char* kMaxFrameTime = "MaxFrameTime";
  static constexpr const char* kFps = "FPS";
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_MONITOR_ANIMAX_METRIC_NAMES_H_
