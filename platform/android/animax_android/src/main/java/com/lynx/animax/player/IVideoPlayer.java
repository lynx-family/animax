// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import com.lynx.animax.base.CalledByNative;

public interface IVideoPlayer {
  @CalledByNative void attachAsset(VideoAsset asset);

  @CalledByNative void setSurface(int texture);

  @CalledByNative boolean updateSurface(int toFrame);

  @CalledByNative float[] getTransformMatrix();

  @CalledByNative
  default void updateOutputFrameSize(int width, int height) {}

  @CalledByNative void destroy();
}
