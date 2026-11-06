// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.UIThreadUtils;

/** Callback for an asynchronous text intrinsic width measurement. */
public interface AnimaXTextMeasureCallback {
  /**
   * Called on the main thread with the untransformed width in original JSON composition units.
   * The width is zero when measurement fails.
   */
  void onComplete(boolean success, float width);

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @CalledByNative
  default void onTextMeasureComplete(boolean success, float width) {
    UIThreadUtils.runOnUiThread(() -> onComplete(success, success ? width : 0f));
  }
}
