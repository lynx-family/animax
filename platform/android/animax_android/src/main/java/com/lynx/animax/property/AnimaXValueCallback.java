// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;

/**
 * Interface for callbacks that can dynamically modify animation property values.
 */
public interface AnimaXValueCallback {
  /**
   * Override this to provide custom values for specific frames.
   * If you haven't set a static value with setValue(), this will be called for each frame.
   *
   * @param frameInfo Information about the current frame
   * @return The value to use for this frame, or null to use the original value
   */
  @Nullable AnimaXValueParam getValue(@NonNull AnimaXFrameInfo frameInfo);

  /**
   * Called by native code to get the value for a specific frame
   */
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @Nullable
  @CalledByNative
  default Object getValueInternal(float startFrame, float endFrame, AnimaXValueParam startValue,
      AnimaXValueParam endValue, float linearProgress, float interpolatedProgress,
      float overallProgress) {
    AnimaXFrameInfo frameInfo = new AnimaXFrameInfo(startFrame, endFrame, startValue, endValue,
        linearProgress, interpolatedProgress, overallProgress);
    return getValue(frameInfo);
  }
}
