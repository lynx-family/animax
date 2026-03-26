// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.player;

import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

import com.lynx.animax.player.IVideoPlayer;
import com.lynx.animax.player.VideoPlayerConfig;
import com.lynx.animax.player.VideoPlayerFactory;
import com.lynx.animax.player.VideoPlayerImpl;
import com.lynx.animax.player.VideoPlayerType;
import com.lynx.animax.service.ServiceRegistry;
import org.junit.Before;
import org.junit.Test;

public class VideoPlayerFactoryTest {
  private ServiceRegistry serviceRegistry;

  @Before
  public void setUp() {
    serviceRegistry = mock(ServiceRegistry.class);
  }

  @Test
  public void testCreateDefaultPlayer() {
    // Arrange
    long nativePlayer = 0L;
    VideoPlayerConfig config = new VideoPlayerConfig(serviceRegistry);
    config.setPlayerType(VideoPlayerType.DEFAULT);

    // Act
    IVideoPlayer player = VideoPlayerFactory.create(nativePlayer, config);

    // Assert
    assertNotNull("Player should not be null", player);
    assertTrue(
        "Player should be an instance of VideoPlayerImpl", player instanceof VideoPlayerImpl);
  }

  @Test
  public void testCreateWithNullConfig() {
    // Arrange
    long nativePlayer = 0L;

    // Act & Assert
    try {
      VideoPlayerFactory.create(nativePlayer, null);
      fail("Expected NullPointerException was not thrown");
    } catch (NullPointerException e) {
      // Expected exception
    }
  }

  @Test
  public void testCreateWithTimeout() {
    // Arrange
    long nativePlayer = 0L;
    VideoPlayerConfig config = new VideoPlayerConfig(serviceRegistry);
    config.setPlayerType(VideoPlayerType.DEFAULT);
    long timeout = 1000L;
    config.setVideoFrameTimeout(timeout);
    config.setSurfaceDestroyTimeout(timeout);

    // Act
    IVideoPlayer player = VideoPlayerFactory.create(nativePlayer, config);

    // Assert
    assertNotNull("Player should not be null", player);
  }
}
