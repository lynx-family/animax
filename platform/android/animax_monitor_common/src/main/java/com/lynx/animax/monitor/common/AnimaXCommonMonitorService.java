// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.monitor.common;

import androidx.annotation.Keep;
import com.bytedance.apm.ApmAgent;
import com.google.auto.service.AutoService;
import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.monitor.AnimaXMonitorUtil;
import com.lynx.animax.monitor.MetricsAndEventStore;
import com.lynx.animax.service.IAnimaXMonitorService;
import com.lynx.animax.service.IAutoRegisterAnimaXService;
import com.lynx.animax.util.AnimaXLog;
import org.json.JSONException;
import org.json.JSONObject;

@Keep
@AutoService(IAutoRegisterAnimaXService.class)
public class AnimaXCommonMonitorService
    implements IAnimaXMonitorService, IAutoRegisterAnimaXService {
  private static final String TAG = "AnimaXCommonMonitorService";

  @Override
  public void reportError(AnimaXErrorParam errorInfo) {
    JSONObject category = new JSONObject();
    try {
      category.put(AnimaXMonitorUtil.KEY_SRC_URL, getUrlHolder().getCurrentUrl());
      category.put(AnimaXMonitorUtil.KEY_MESSAGE, errorInfo.getErrorMessage());
      category.put(AnimaXMonitorUtil.KEY_CODE, errorInfo.getErrorCode());
    } catch (JSONException e) {
      AnimaXLog.e(TAG, "JSON exception in AnimaXCommonMonitorService.reportError: " + e.toString());
      return;
    }

    AnimaXLog.i(TAG, "reportError category: " + category);
    ApmAgent.monitorEvent("animax_native_error", category, null, null);
  }

  @Override
  public void reportPerformanceMetrics(MetricsAndEventStore metrics) {
    if (metrics == null) {
      AnimaXLog.e(TAG, "report failed, metrics is null.");
      return;
    }

    JSONObject category = metrics.getCategoryAsJSON();
    try {
      category.put(AnimaXMonitorUtil.KEY_SRC_URL, getUrlHolder().getCurrentUrl());
    } catch (JSONException e) {
      AnimaXLog.e(TAG, "JSON exception in AnimaXCommonMonitorService.reportError: " + e.toString());
      return;
    }
    JSONObject metric = metrics.getMetricsAsJSON();

    AnimaXLog.i(TAG, "reportPerformance category: " + category + ", metric: " + metric);
    ApmAgent.monitorEvent("animax_native_performance", category, metric, null);
  }

  @Override
  public Class<? extends IAnimaXMonitorService> getServiceClass() {
    return IAnimaXMonitorService.class;
  }
}
