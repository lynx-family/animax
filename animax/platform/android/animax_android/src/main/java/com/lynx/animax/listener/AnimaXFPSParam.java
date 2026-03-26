// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.listener;

import androidx.annotation.NonNull;
import java.util.Map;

public class AnimaXFPSParam extends AnimaXParam {
  private static final String KEY_FPS = "fps";
  private static final String KEY_MAX_DROP_RATE = "max_drop_rate";

  /**
   * Constructor to create an event containing FPS information.
   * @param originParams A map containing the original parameters with event information.
   */
  public AnimaXFPSParam(@NonNull Map<String, Object> originParams) {
    super(originParams);
  }

  /**
   * Retrieves the frames per second from the event.
   * @return The FPS count as a float.
   */
  public float getFPS() {
    return getFloat(KEY_FPS);
  }

  /**
   * Retrieves the maximum drop rate of frames from the event.
   * @return The maximum drop rate as a float.
   */
  public float getMaxDropRate() {
    return getFloat(KEY_MAX_DROP_RATE);
  }
}
