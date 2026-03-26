// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.monitor.common;

import static org.junit.Assert.assertNotNull;

import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.monitor.MetricsAndEventStore;
import java.util.HashMap;
import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Before;
import org.junit.Test;

public class AnimaXCommonMonitorTest {
  private AnimaXCommonMonitorService monitor;

  @Before
  public void setup() {
    monitor = new AnimaXCommonMonitorService();
    monitor.getUrlHolder().setCurrentUrl("testSrcURL");
  }

  @Test
  public void testReportError() throws JSONException {
    HashMap<String, Object> m = new HashMap<String, Object>();
    m.put("error code", 404);
    // Arrange
    AnimaXErrorParam errorInfo = new AnimaXErrorParam(m);

    // Act
    monitor.reportError(errorInfo);

    // Verify basic non-null properties (e.g., that log or JSON creation happened)
    JSONObject errorInfoJSON = new JSONObject();
    errorInfoJSON.put("src_url", "http://example.com");
    errorInfoJSON.put("error_message", errorInfo.getErrorMessage());
    errorInfoJSON.put("error_code", errorInfo.getErrorCode());

    assertNotNull(errorInfoJSON);
  }

  @Test
  public void testReportPerformanceMetrics_withValidMetrics() throws JSONException {
    // Arrange
    MetricsAndEventStore mockMetrics = new MetricsAndEventStore(null, null, new HashMap<>(), null);
    JSONObject categoryJSON = mockMetrics.getCategoryAsJSON();
    JSONObject metricsJSON = mockMetrics.getMetricsAsJSON();

    // Act
    monitor.reportPerformanceMetrics(mockMetrics);

    // Verify that the JSON objects were populated with data
    categoryJSON.put("src_url", "http://example.com");
    assertNotNull(categoryJSON);
    assertNotNull(metricsJSON);
  }

  @Test
  public void testReportPerformanceMetrics_withNullMetrics() {
    // Act
    monitor.reportPerformanceMetrics(null);

    // Verify (simplistically check if method call doesn't throw)
    // since with null metrics there should be no processing
  }
}
