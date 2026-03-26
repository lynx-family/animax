// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax;

import android.view.Choreographer;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.UIThreadUtils;

public class VSyncMonitor {
  @CalledByNative
  public static void requestVSync() {
    // Call from AnimaX_Main thread to UI main thread.
    UIThreadUtils.postAsynchronousAtFrontOfQueueOnUiThread(() -> {
      Choreographer.getInstance().postFrameCallback(new Choreographer.FrameCallback() {
        @Override
        public void doFrame(long frameTimeNanos) {
          VSyncMonitor.invokeNativeCallback(frameTimeNanos);
        }
      });
    });
  }

  private static void invokeNativeCallback(long frameTimeNanos) {
    nativeInvokeCallback(frameTimeNanos);
  }

  private static native void nativeInvokeCallback(long frameTimeNanos);
}
