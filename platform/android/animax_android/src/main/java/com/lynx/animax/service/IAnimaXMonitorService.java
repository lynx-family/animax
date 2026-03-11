// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.service;

import androidx.annotation.Keep;
import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.monitor.MetricsAndEventStore;

/**
 * Service interface for monitoring AnimaX events and metrics.
 */
@Keep
public interface IAnimaXMonitorService extends IAnimaXService {
  final class UrlHolder {
    private String mCurrentUrl = "unknown";

    public String getCurrentUrl() {
      return mCurrentUrl;
    }

    public void setCurrentUrl(String url) {
      if (url != null) {
        this.mCurrentUrl = url;
      }
    }
  }

  UrlHolder URL_HOLDER = new UrlHolder();

  default UrlHolder getUrlHolder() {
    return URL_HOLDER;
  }

  void reportError(AnimaXErrorParam errorInfo);
  void reportPerformanceMetrics(MetricsAndEventStore metrics);
}
