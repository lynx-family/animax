// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import android.util.Log;
import com.lynx.animax.util.AnimaX;

public class AnimaXLog {
  private static final String TAG = "[AnimaX]";

  public enum LogLevel { VERBOSE, DEBUG, INFO, WARN, ERROR }

  public static void v(String tag, String msg) {
    log(LogLevel.VERBOSE, tag, msg);
  }

  public static void d(String tag, String msg) {
    log(LogLevel.DEBUG, tag, msg);
  }

  public static void i(String tag, String msg) {
    log(LogLevel.INFO, tag, msg);
  }

  public static void w(String tag, String msg) {
    log(LogLevel.WARN, tag, msg);
  }

  public static void e(String tag, String msg) {
    log(LogLevel.ERROR, tag, msg);
  }

  private static void log(LogLevel level, String tag, String msg) {
    if (AnimaX.inst().hasInitialized()) {
      nativeLog(level.ordinal(), tag, msg);
    } else {
      Log.e(TAG, "AnimaX is not initialized, log will be ignored: " + msg);
    }
  }

  private static native void nativeLog(int level, String tag, String msg);
}
