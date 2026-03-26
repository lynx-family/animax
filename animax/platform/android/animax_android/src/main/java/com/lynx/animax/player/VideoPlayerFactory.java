// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import com.lynx.animax.service.IAnimaXVideoService;
import com.lynx.animax.util.AnimaXLog;

public class VideoPlayerFactory {
  private static final String TAG = "VideoPlayerFactory";

  public static IVideoPlayer create(long nativePlayer, VideoPlayerConfig config) {
    if (config.getPlayerType() == VideoPlayerType.CUSTOM) {
      IAnimaXVideoService service =
          config.getServiceRegistry().getService(IAnimaXVideoService.class);
      if (service != null) {
        return service.createVideoPlayer(nativePlayer, config);
      }
      AnimaXLog.e(TAG, "IAnimaXVideoService not found, use default player");
    }
    return new VideoPlayerImpl(nativePlayer, config);
  }
}
