// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import androidx.annotation.Nullable;

public class UIThreadUtils {
  @Nullable private static volatile Handler sMainHandler;

  public static Handler getUiThreadHandler() {
    if (sMainHandler == null) {
      synchronized (UIThreadUtils.class) {
        if (sMainHandler == null) {
          sMainHandler = new Handler(Looper.getMainLooper());
        }
      }
    }
    return sMainHandler;
  }

  public static boolean isOnUiThread() {
    return Looper.getMainLooper().getThread() == Thread.currentThread();
  }

  public static void assertOnUiThread() {
    if (!isOnUiThread()) {
      throw new AssertionError("Expected to run on UI thread!");
    }
  }

  public static void assertNotOnUiThread() {
    if (isOnUiThread()) {
      throw new AssertionError("Expected not to run on UI thread!");
    }
  }

  public static void runOnUiThread(Runnable runnable) {
    getUiThreadHandler().post(runnable);
  }

  public static void postAtFrontOfQueueOnUiThread(Runnable runnable) {
    getUiThreadHandler().postAtFrontOfQueue(runnable);
  }

  public static void runOnUiThreadImmediately(Runnable runnable) {
    if (isOnUiThread()) {
      runnable.run();
    } else {
      runOnUiThread(runnable);
    }
  }

  public static void postAsynchronousAtFrontOfQueueOnUiThread(Runnable runnable) {
    Handler handler = getUiThreadHandler();
    Message message = Message.obtain(handler, runnable);
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP_MR1) {
      message.setAsynchronous(true);
    }
    handler.sendMessageAtFrontOfQueue(message);
  }
}
