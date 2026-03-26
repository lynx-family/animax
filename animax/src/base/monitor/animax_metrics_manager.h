// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_MONITOR_ANIMAX_METRICS_MANAGER_H_
#define ANIMAX_SRC_BASE_MONITOR_ANIMAX_METRICS_MANAGER_H_

#include "base/include/lynx_actor.h"
#include "include/base/animax_metrics.h"
#include "src/base/monitor/timestamp_recorder.h"

namespace lynx {
namespace animax {

class CompositionModelPipeline;
class AnimaXRenderer;
class AnimaXCompositionLoader;
class AnimaXMainController;

using TimeCost = uint32_t;

class AnimaXMetricsManager
    : public std::enable_shared_from_this<AnimaXMetricsManager> {
 public:
  AnimaXMetricsManager(
      std::shared_ptr<shell::LynxActor<AnimaXCompositionLoader>> loader_actor,
      std::shared_ptr<shell::LynxActor<AnimaXRenderer>> renderer_actor,
      std::shared_ptr<shell::LynxActor<AnimaXMainController>> controller_actor);
  virtual ~AnimaXMetricsManager() = default;

  void Collect(ExternalMetricsReadyCallback callback);

 private:
  void CollectFromLoader(TimestampArray renderer_timestamps,
                         MetricsMap renderer_metrics,
                         ExternalMetricsReadyCallback callback);
  void CollectFromController(TimestampArray renderer_timestamps,
                             MetricsMap renderer_metrics,
                             TimestampArray resource_timestamps,
                             ExternalMetricsReadyCallback callback);

  // Hold the shared_ptr to the actors to avoid the actor being destroyed
  // before the metrics collection.
  std::shared_ptr<shell::LynxActor<AnimaXCompositionLoader>> loader_actor_;
  std::shared_ptr<shell::LynxActor<AnimaXRenderer>> renderer_actor_;
  std::shared_ptr<shell::LynxActor<AnimaXMainController>> controller_actor_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_MONITOR_ANIMAX_METRICS_MANAGER_H_
