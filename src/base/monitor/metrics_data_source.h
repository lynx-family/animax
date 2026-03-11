// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_MONITOR_METRICS_DATA_SOURCE_H_
#define ANIMAX_SRC_BASE_MONITOR_METRICS_DATA_SOURCE_H_

#include "include/base/animax_metrics.h"
#include "src/base/monitor/timestamp_recorder.h"

namespace lynx {
namespace animax {

/**
 * This interface defines the basic contract for components that need to
 * provide metrics data. Components implementing this interface
 * can handle their own metrics collection independently.
 */
class MetricsDataSource {
 public:
  virtual ~MetricsDataSource() = default;

  /**
   * Records a timestamp for the specified event type.
   *
   * @param type The type of event to record a timestamp for.
   */
  virtual void Trace(TraceEventType type) = 0;

  /**
   * Exports collected timestamp data.
   *
   * @return A vector containing the collected timestamps.
   */
  virtual TimestampArray ExportTimestamps() const = 0;

  /**
   * Exports collected metrics data as a map.
   *
   * @return A map containing the collected metrics data.
   */
  virtual MetricsMap ExportMetricsMap() const { return MetricsMap{}; }
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_MONITOR_METRICS_DATA_SOURCE_H_
