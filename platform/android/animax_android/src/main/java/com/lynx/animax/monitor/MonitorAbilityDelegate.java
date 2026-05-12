// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.monitor;

import androidx.annotation.NonNull;
import com.lynx.animax.AnimaXPlayer;
import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.listener.AnimaXParam;
import com.lynx.animax.listener.AnimationListenerAdapter;
import com.lynx.animax.service.IAnimaXMonitorService;
import com.lynx.animax.service.ServiceRegistry;
import com.lynx.animax.ui.IAnimaXPlayer;
import com.lynx.animax.util.AnimaXMetricsCallback;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Delegate class that handles monitoring responsibilities for AnimaX abilities.
 * This class is responsible for collecting and reporting metrics, errors, and performance data
 * on behalf of BaseAbility.
 *
 * <p>Key responsibilities:
 * - Monitors and reports animation performance metrics
 * - Tracks and reports error events
 * - Manages URL updates for monitoring context
 * - Handles completion and release events
 */
public class MonitorAbilityDelegate extends AnimationListenerAdapter {
  private final ServiceRegistry mServiceRegistry;
  private WeakReference<IAnimaXPlayer> mAnimaXPlayer;
  private boolean mHasReportedFirstPlay = false;

  private final Map<String, Object> mPlatformReportItems = new ConcurrentHashMap<>();
  private long mLastPlayReportTimestamp;

  // Count when animation is played, resumed or repeated.
  private int mPlayCount;
  // Count when animation is played or resumed. when comes to a threshold, it will trigger a onPlay
  // report.
  private int mPlayTriggerCount;

  private final long mStayDurationStartTimestamp;

  public MonitorAbilityDelegate(@NonNull ServiceRegistry serviceRegistry) {
    mServiceRegistry = serviceRegistry;
    mStayDurationStartTimestamp = System.currentTimeMillis();
    mLastPlayReportTimestamp = mStayDurationStartTimestamp;
  }

  public void setAnimaXPlayer(@NonNull AnimaXPlayer player) {
    mAnimaXPlayer = new WeakReference<>(player);
  }

  /**
   * Set which display mode the animation is at.
   *
   * @param mode "image" or "surface"
   */
  public void setDisplayMode(@NonNull String mode) {
    mPlatformReportItems.put(AnimaXMonitorUtil.PLATFORM_KEY_DISPLAY_MODE, mode);
  }

  /**
   * Set the tag for experiment.
   *
   * @param tag tag for experiment.
   */
  public void setTag(@NonNull String tag) {
    mPlatformReportItems.put(AnimaXMonitorUtil.PLATFORM_KEY_TAG, tag);
  }

  /**
   * Handles error events by reporting them through the monitor service.
   *
   * @param param Error parameters containing error details
   */
  @Override
  public void onError(AnimaXErrorParam param) {
    IAnimaXMonitorService monitor = mServiceRegistry.getService(IAnimaXMonitorService.class);
    if (monitor != null) {
      monitor.reportError(param);
    }
  }

  /**
   * Handles repeat events by reporting onPlay metrics.
   *
   * @param param Repeat parameters
   */
  @Override
  public void onRepeat(AnimaXParam param) {
    reportOnPlay(false);
  }

  /**
   * Updates the current URL in the monitor service.
   *
   * @param url The URL to be updated
   */
  public void updateUrl(String url) {
    IAnimaXMonitorService monitor = mServiceRegistry.getService(IAnimaXMonitorService.class);
    if (url != null && monitor != null) {
      monitor.setCurrentUrl(url);
    }
  }

  /**
   * Handles release events by reporting final metrics.
   * Should be called when the ability is being released or destroyed.
   */
  public void onRelease() {
    mPlatformReportItems.put(AnimaXMonitorUtil.PLATFORM_KEY_STAY_DURATION,
        System.currentTimeMillis() - mStayDurationStartTimestamp);
    reportPerformance(AnimaXMonitorUtil.TRIGGER_ON_RELEASE);
  }

  /**
   * Handles resume method by reporting onPlay metrics.
   */
  public void onResume() {
    reportOnPlay();
  }

  /**
   * Handles playSegment method by reporting onPlay metrics.
   */
  public void onPlaySegment() {
    reportOnPlay();
  }

  /**
   * Handles play method by reporting onPlay metrics.
   */
  public void onPlay() {
    reportOnPlay();
  }

  private void reportOnPlay() {
    reportOnPlay(true);
  }

  /**
   * Called when onRepeat, resume, play, or play segment.
   * If the last report was within MinimumIntervalTime and the cumulative play count is less than
   * MaximumPlayCount, do not report.
   */
  private void reportOnPlay(boolean enableTrigger) {
    mPlayCount++;
    if (enableTrigger) {
      mPlayTriggerCount++;
    }
    long currentTime = System.currentTimeMillis();
    if (!mHasReportedFirstPlay || mPlayTriggerCount >= AnimaXMonitorUtil.MAXIMUM_PLAY_COUNT
        || currentTime - mLastPlayReportTimestamp >= AnimaXMonitorUtil.MINIMUM_INTERVAL_TIME) {
      reportPerformance(AnimaXMonitorUtil.TRIGGER_ON_PLAY);
      mHasReportedFirstPlay = true;
      mLastPlayReportTimestamp = currentTime;
    }
  }

  private void reportPerformance(String trigger) {
    IAnimaXMonitorService monitor = mServiceRegistry.getService(IAnimaXMonitorService.class);
    IAnimaXPlayer player = mAnimaXPlayer != null ? mAnimaXPlayer.get() : null;
    if (player == null || monitor == null) {
      return;
    }
    HashMap<String, Object> map = new HashMap<>(mPlatformReportItems);
    if (mPlayCount > 0) {
      map.put(AnimaXMonitorUtil.PLATFORM_KEY_PLAY_COUNT, mPlayCount);
      mPlayCount = 0;
      mPlayTriggerCount = 0;
    }
    player.getMetricsAsync(new AnimaXMetricsCallback(monitor, trigger, map));
  }
}
