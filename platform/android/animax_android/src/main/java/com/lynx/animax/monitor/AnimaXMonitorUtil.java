// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.monitor;
import android.net.Uri;
import com.lynx.animax.util.AnimaXLog;
import java.util.Map;
import org.json.JSONException;
import org.json.JSONObject;

public final class AnimaXMonitorUtil {
  public static final String TAG = "AnimaXMonitor";
  public static final String PERFORMANCE_EVENT_NAME = "animax_performance";
  public static final String ERROR_EVENT_NAME = "animax_error";

  public static final String CLASS_PERF_METRICS = "PerfMetrics";
  public static final String KEY_FPS = "FPS";
  public static final String KEY_SRC_URL = "src_url";
  public static final String KEY_PAGE_URL = "page_url";
  public static final String KEY_CODE = "code";
  public static final String KEY_MESSAGE = "msg";
  public static final String KEY_INTEGRATION_TYPE = "integration_type";

  // names for triggers used in different circumstances
  // Triggers, along with events and metrics, will be reported to the outside world.
  public static final String TRIGGER_ON_RELEASE = "onRelease";
  public static final String TRIGGER_ON_PLAY = "onPlay";
  public static final String TRIGGER_DEFAULT = "default";

  public static final String PLATFORM_KEY_PLAY_COUNT = "play_count";
  public static final String PLATFORM_KEY_STAY_DURATION = "stay_duration";
  public static final String PLATFORM_KEY_DISPLAY_MODE = "display_mode";
  public static final String PLATFORM_KEY_TAG = "tag";

  public static final long MINIMUM_INTERVAL_TIME = 5 * 60 * 1000;
  public static final long MAXIMUM_PLAY_COUNT = 5;

  public static String clearUrlQuery(String originUrl) {
    try {
      if (originUrl != null && !originUrl.isEmpty()) {
        return Uri.parse(originUrl).buildUpon().clearQuery().build().toString();
      }
    } catch (UnsupportedOperationException e) {
      AnimaXLog.e(TAG, "clearUrlQuery failed, error: " + e.toString());
    }
    return "unknown";
  }

  public static JSONObject convertHashMapIntoJSON(Map<String, Object> map) {
    JSONObject jsonObject = new JSONObject();

    try {
      for (Map.Entry<String, Object> entry : map.entrySet()) {
        jsonObject.put(entry.getKey(), entry.getValue());
      }
    } catch (JSONException e) {
      AnimaXLog.e(TAG, "Converting hashMap to JSON failed, error: " + e.toString());
    }

    return jsonObject;
  }
}
