// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Process;
import com.lynx.animax.util.AnimaXLog;

public class CodecThreadManager {
  private static final String TAG = "CodecThreadManager";
  private HandlerThread mCodecThread;
  private Handler mCodecThreadHandler;
  private static final String DEFAULT_THREAD_NAME = "Animax_Codec";

  private static CodecThreadManager sInstance;

  private CodecThreadManager() {}

  public static synchronized CodecThreadManager getInstance() {
    if (sInstance == null) {
      sInstance = new CodecThreadManager();
      sInstance.init();
    }
    return sInstance;
  }

  private void init() {
    if (mCodecThread == null) {
      mCodecThread = new HandlerThread(DEFAULT_THREAD_NAME, Process.THREAD_PRIORITY_DISPLAY);
      mCodecThread.start();
      mCodecThreadHandler = new Handler(mCodecThread.getLooper());
    }
  }

  public void runNowOrPostToCodecThread(Runnable r) {
    if (mCodecThreadHandler == null) {
      AnimaXLog.e(TAG, "Attempted to post to codec thread after it was released or not alive.");
      return;
    }
    if (Looper.myLooper() == mCodecThreadHandler.getLooper()) {
      r.run();
    } else {
      mCodecThreadHandler.post(r);
    }
  }

  /**
   * Posts a runnable to the front of the codec thread queue and clears all pending messages.
   * @param r The runnable to execute
   */
  public void postAtFrontAndClearQueue(Runnable r) {
    if (mCodecThreadHandler == null) {
      AnimaXLog.e(TAG, "Attempted to post to codec thread after it was released or not alive.");
      return;
    }
    // Clear all pending messages and callbacks
    mCodecThreadHandler.removeCallbacksAndMessages(null);
    // Post the new runnable at the front of the queue
    mCodecThreadHandler.postAtFrontOfQueue(r);
  }

  public void ensureOnCodecThread() {
    if (mCodecThreadHandler == null || Looper.myLooper() != mCodecThreadHandler.getLooper()) {
      throw new IllegalStateException("Must be called on codec thread (" + DEFAULT_THREAD_NAME
          + "), current: " + Thread.currentThread().getName());
    }
  }
}
