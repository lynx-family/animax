// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/monitor/animax_metrics_manager.h"

#include "src/base/monitor/animax_metric_names.h"
#include "src/player/animax_composition_loader.h"
#include "src/player/animax_renderer.h"

namespace lynx {
namespace animax {

namespace {

TimeCost CalculateInterval(TraceEventType start, TraceEventType end,
                           const TimestampArray& timestamp_array) {
  auto start_time = timestamp_array[static_cast<size_t>(start)];
  auto end_time = timestamp_array[static_cast<size_t>(end)];

  if (end_time < start_time || start_time == 0 || end_time == 0) {
    return 0;
  }

  return static_cast<TimeCost>(end_time - start_time);
}

void CopyTimeStampArray(const TimestampArray& src, TimestampArray& dst) {
  for (size_t i = 0; i < src.size(); i++) {
    if (src[i] > 0) {
      dst[i] = src[i];
    }
  }
}

MetricsMap CalculateMetricsFromTimestamps(const TimestampArray& timestamps) {
  MetricsMap metrics_to_export;

  auto animax_fcp =
      CalculateInterval(TraceEventType::kRequestCompositionStart,
                        TraceEventType::kRenderFrameEnd, timestamps);
  if (animax_fcp > 0) {
    metrics_to_export[AnimaXMetricNames::kAnimaXFCP] = animax_fcp;
  }

  auto time_cost_of_resource_preparation =
      CalculateInterval(TraceEventType::kRequestCompositionStart,
                        TraceEventType::kPrepareAssetsEnd, timestamps);
  if (time_cost_of_resource_preparation > 0) {
    metrics_to_export[AnimaXMetricNames::kPrepareResourceTime] =
        time_cost_of_resource_preparation;
  }

  auto time_cost_of_composition_preparation =
      CalculateInterval(TraceEventType::kRequestCompositionStart,
                        TraceEventType::kParseCompositionEnd, timestamps);
  if (time_cost_of_composition_preparation > 0) {
    metrics_to_export[AnimaXMetricNames::kPrepareCompositionTime] =
        time_cost_of_composition_preparation;
  }

  auto time_cost_of_assets_preparation =
      CalculateInterval(TraceEventType::kPrepareAssetsStart,
                        TraceEventType::kPrepareAssetsEnd, timestamps);
  if (time_cost_of_assets_preparation > 0) {
    metrics_to_export[AnimaXMetricNames::kPrepareAssetsTime] =
        time_cost_of_assets_preparation;
  }

  auto first_frame_time =
      CalculateInterval(TraceEventType::kRenderFrameStart,
                        TraceEventType::kRenderFrameEnd, timestamps);
  if (first_frame_time > 0) {
    metrics_to_export[AnimaXMetricNames::kFirstFrameTime] = first_frame_time;
  }

  return metrics_to_export;
}
}  // namespace

void AnimaXMetricsManager::Collect(ExternalMetricsReadyCallback on_completion) {
  if (!renderer_actor_) {
    on_completion(MetricsMap());
    return;
  }
  renderer_actor_->Act(
      [weak_this = weak_from_this(),
       on_completion = std::move(on_completion)](auto& renderer) mutable {
        auto renderer_timestamps = renderer->ExportTimestamps();
        auto renderer_metrics = renderer->ExportMetricsMap();

        if (auto shared_this = weak_this.lock()) {
          shared_this->CollectFromLoader(std::move(renderer_timestamps),
                                         std::move(renderer_metrics),
                                         std::move(on_completion));
        } else {
          on_completion(MetricsMap());
        }
      });
}

void AnimaXMetricsManager::CollectFromLoader(
    TimestampArray renderer_timestamps, MetricsMap renderer_metrics,
    ExternalMetricsReadyCallback on_completion) {
  if (!loader_actor_) {
    on_completion(MetricsMap());
    return;
  }
  loader_actor_->Act(
      [weak_this = weak_from_this(),
       renderer_timestamps = std::move(renderer_timestamps),
       renderer_metrics = std::move(renderer_metrics),
       on_completion = std::move(on_completion)](auto& loader) mutable {
        auto resource_timestamps = loader->ExportTimestamps();

        if (auto shared_this = weak_this.lock()) {
          shared_this->CollectFromController(
              std::move(renderer_timestamps), std::move(renderer_metrics),
              std::move(resource_timestamps), std::move(on_completion));
        } else {
          on_completion(MetricsMap());
        }
      });
}

void AnimaXMetricsManager::CollectFromController(
    TimestampArray renderer_timestamps, MetricsMap renderer_metrics,
    TimestampArray resource_timestamps,
    ExternalMetricsReadyCallback on_completion) {
  if (!controller_actor_) {
    on_completion(MetricsMap());
    return;
  }
  controller_actor_->Act([renderer_timestamps = std::move(renderer_timestamps),
                          renderer_metrics = std::move(renderer_metrics),
                          resource_timestamps = std::move(resource_timestamps),
                          on_completion = std::move(on_completion)](
                             auto& controller) mutable {
    TimestampArray combined_timestamps = renderer_timestamps;
    CopyTimeStampArray(resource_timestamps, combined_timestamps);

    auto combined_metrics = CalculateMetricsFromTimestamps(combined_timestamps);
    combined_metrics.insert(renderer_metrics.begin(), renderer_metrics.end());

    on_completion(std::move(combined_metrics));
  });
}

AnimaXMetricsManager::AnimaXMetricsManager(
    std::shared_ptr<shell::LynxActor<AnimaXCompositionLoader>> loader_actor,
    std::shared_ptr<shell::LynxActor<AnimaXRenderer>> renderer_actor,
    std::shared_ptr<shell::LynxActor<AnimaXMainController>> controller_actor) {
  loader_actor_ = std::move(loader_actor);
  renderer_actor_ = std::move(renderer_actor);
  controller_actor_ = std::move(controller_actor);
}

}  // namespace animax
}  // namespace lynx
