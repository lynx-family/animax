// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.monitor;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.bridge.JavaOnlyMap;
import com.lynx.animax.base.bridge.ReadableMap;
import java.util.HashMap;
import java.util.Map;
import org.json.JSONObject;

@RestrictTo(RestrictTo.Scope.LIBRARY)
public class MetricsAndEventStore {
  private final ReadableMap mMetricsMap;
  private final ReadableMap mCategoryMap;

  public MetricsAndEventStore(ReadableMap metricsMap, ReadableMap eventsMap,
      @NonNull Map<String, Object> platformMap, String trigger) {
    Map<String, Object> metrics = metricsMap == null ? new HashMap<>() : metricsMap.asHashMap();
    Map<String, Object> categories = eventsMap == null ? new HashMap<>() : eventsMap.asHashMap();
    for (Map.Entry<String, Object> e : platformMap.entrySet()) {
      String k = e.getKey();
      Object v = e.getValue();
      if (v == null)
        continue;
      if (v instanceof Number) {
        metrics.put(k, v);
      } else if (v instanceof String || v instanceof Boolean) {
        categories.put(k, v);
      }
    }
    if (trigger != null) {
      categories.put(AnimaXMonitorUtil.TRIGGER_DEFAULT, trigger);
    }
    mMetricsMap = JavaOnlyMap.from(metrics);
    mCategoryMap = JavaOnlyMap.from(categories);
  }

  public double getFps() {
    return mMetricsMap.getDouble(AnimaXMonitorUtil.KEY_FPS, 0);
  }

  public Map<String, Object> getMetricsAsMap() {
    return new HashMap<>(mMetricsMap.asHashMap());
  }

  public JSONObject getMetricsAsJSON() {
    return AnimaXMonitorUtil.convertHashMapIntoJSON(getMetricsAsMap());
  }

  public Map<String, Object> getCategoryAsMap() {
    return new HashMap<>(mCategoryMap.asHashMap());
  }

  public JSONObject getCategoryAsJSON() {
    return AnimaXMonitorUtil.convertHashMapIntoJSON(getCategoryAsMap());
  }

  @Override
  public String toString() {
    StringBuilder res = new StringBuilder();

    res.append("PerfMetrics{");
    final HashMap<String, Object> metricsMap = mMetricsMap.asHashMap();
    for (Map.Entry<String, Object> entry : metricsMap.entrySet()) {
      if (entry.getValue() != null) {
        res.append(entry.getKey()).append("=").append(entry.getValue()).append(",");
      }
    }

    final HashMap<String, Object> eventsMap = mCategoryMap.asHashMap();
    for (Map.Entry<String, Object> entry : eventsMap.entrySet()) {
      if (entry.getValue() != null) {
        res.append(entry.getKey()).append("=").append(entry.getValue()).append(",");
      }
    }
    res.append("}");

    return res.toString();
  }
}
