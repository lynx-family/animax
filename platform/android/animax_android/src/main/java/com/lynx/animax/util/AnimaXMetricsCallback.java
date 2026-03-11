// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.base.bridge.ReadableMap;
import com.lynx.animax.monitor.MetricsAndEventStore;
import com.lynx.animax.service.IAnimaXMonitorService;
import java.util.Map;

@RestrictTo(RestrictTo.Scope.LIBRARY)
public class AnimaXMetricsCallback {
  private final IAnimaXMonitorService mMonitor;
  private final String mTrigger;
  @NonNull private final Map<String, Object> mPlatform;

  public AnimaXMetricsCallback(@NonNull IAnimaXMonitorService monitor, String trigger,
      @NonNull Map<String, Object> platform) {
    mMonitor = monitor;
    mTrigger = trigger;
    mPlatform = platform;
  }

  @CalledByNative
  public void onMetricsReady(ReadableMap metrics, ReadableMap events) {
    mMonitor.reportPerformanceMetrics(
        new MetricsAndEventStore(metrics, events, mPlatform, mTrigger));
  }
}
