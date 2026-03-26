// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import androidx.annotation.NonNull;
import com.lynx.animax.service.ServiceRegistry;

public class VideoPlayerConfig {
  @NonNull private final ServiceRegistry mServiceRegistry;
  private VideoPlayerType mPlayerType = VideoPlayerType.DEFAULT;
  private long mVideoFrameTimeout = 0;
  private long mSurfaceDestroyTimeout = 0;

  public VideoPlayerConfig(@NonNull ServiceRegistry serviceRegistry) {
    this.mServiceRegistry = serviceRegistry;
  }

  /**
   * Sets the player type for the video player.
   * @param playerType The type of video player to use.
   */
  public void setPlayerType(VideoPlayerType playerType) {
    this.mPlayerType = playerType;
  }

  /**
   * Sets the video frame timeout for the video player.
   * @param videoFrameTimeout The timeout in milliseconds for each video frame.
   */
  public void setVideoFrameTimeout(long videoFrameTimeout) {
    this.mVideoFrameTimeout = videoFrameTimeout;
  }

  /**
   * Sets the surface destroy timeout for the video player.
   * @param surfaceDestroyTimeout The timeout in milliseconds for the surface to be destroyed.
   */
  public void setSurfaceDestroyTimeout(long surfaceDestroyTimeout) {
    this.mSurfaceDestroyTimeout = surfaceDestroyTimeout;
  }

  /**
   * Gets the player type.
   * @return The VideoPlayerType configured for this player.
   */
  public VideoPlayerType getPlayerType() {
    return mPlayerType;
  }

  /**
   * Gets the video frame timeout.
   * @return The video frame timeout in milliseconds.
   */
  public long getVideoFrameTimeout() {
    return mVideoFrameTimeout;
  }

  /**
   * Gets the surface destroy timeout.
   * @return The surface destroy timeout in milliseconds.
   */
  public long getSurfaceDestroyTimeout() {
    return mSurfaceDestroyTimeout;
  }

  /**
   * Gets the service registry.
   * @return The service registry configured for this player.
   */
  public ServiceRegistry getServiceRegistry() {
    return mServiceRegistry;
  }
}
