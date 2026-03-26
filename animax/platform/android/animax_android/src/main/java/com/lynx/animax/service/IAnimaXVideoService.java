// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import androidx.annotation.Keep;
import com.lynx.animax.player.IVideoPlayer;
import com.lynx.animax.player.VideoPlayerConfig;

/**
 * Service interface for creating custom video players.
 * Provides a method to create an IVideoPlayer instance with the given parameters.
 */
@Keep
public interface IAnimaXVideoService extends IAnimaXService {
  /**
   * Creates a custom video player instance.
   *
   * @param nativePlayer The native player handle
   * @param config The video player configuration
   * @return An IVideoPlayer instance, or null if creation is not handled
   */
  IVideoPlayer createVideoPlayer(long nativePlayer, VideoPlayerConfig config);
}
