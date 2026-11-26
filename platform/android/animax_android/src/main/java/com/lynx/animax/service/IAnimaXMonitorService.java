// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.service;

import androidx.annotation.Keep;
import androidx.annotation.RestrictTo;
import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.monitor.MetricsAndEventStore;
import java.util.Map;

/**
 * Service interface for monitoring AnimaX events and metrics.
 */
@Keep
public interface IAnimaXMonitorService extends IAnimaXService {
  /** Optional per-request reporting for remote resources consumed by a player. */
  interface ResourceLoadReporter {
    void reportResourceLoad(Map<String, Object> properties);
  }

  // Only for XElement built against older SDKs. Remove once it uses ResourceLoadReporter.
  @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
  default boolean useResourceLoadReportingSetting() {
    return false;
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
  default void reportResourceLoad(Map<String, Object> properties) {}

  void setCurrentUrl(String url);
  void reportError(AnimaXErrorParam errorInfo);
  void reportPerformanceMetrics(MetricsAndEventStore metrics);
}
