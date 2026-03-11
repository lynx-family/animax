// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

public class VideoPlayerFactory {
  public static IVideoPlayer create(long nativePlayer, VideoPlayerConfig config) {
    switch (config.getPlayerType()) {
      case CUSTOM:
        // todo(aiyongbiao.rick): create from IAnimaXVideoService
      case DEFAULT:
      default:
        return new VideoPlayerImpl(nativePlayer, config);
    }
  }
}
