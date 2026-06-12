// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Process;
import com.lynx.animax.util.AnimaXLog;
import java.util.concurrent.atomic.AtomicInteger;

public class CodecThreadManager {
  private static final String TAG = "CodecThreadManager";
  private static final String DEFAULT_THREAD_NAME = "Animax_Codec";
  private static final int DEFAULT_THREAD_INDEX = 0;
  private static final int MAX_CODEC_THREAD_COUNT = 6;

  private HandlerThread[] mCodecThreads;
  private Handler[] mCodecThreadHandlers;
  private final AtomicInteger mNextThreadIndex = new AtomicInteger(0);
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
    if (mCodecThreadHandlers == null) {
      mCodecThreads = new HandlerThread[MAX_CODEC_THREAD_COUNT];
      mCodecThreadHandlers = new Handler[MAX_CODEC_THREAD_COUNT];
      getHandler(DEFAULT_THREAD_INDEX);
    }
  }

  public void runNowOrPostToCodecThread(int threadIndex, Runnable r) {
    Handler codecThreadHandler = getHandler(threadIndex);
    if (codecThreadHandler == null) {
      AnimaXLog.e(TAG, "Attempted to post to codec thread after it was released or not alive.");
      return;
    }
    if (Looper.myLooper() == codecThreadHandler.getLooper()) {
      r.run();
    } else {
      codecThreadHandler.post(r);
    }
  }

  /**
   * Posts a runnable to the front of the codec thread queue and clears all pending messages.
   * @param r The runnable to execute
   */
  public void postAtFrontAndClearQueue(int threadIndex, Runnable r) {
    Handler codecThreadHandler = getHandler(threadIndex);
    if (codecThreadHandler == null) {
      AnimaXLog.e(TAG, "Attempted to post to codec thread after it was released or not alive.");
      return;
    }
    // Clear all pending messages and callbacks
    codecThreadHandler.removeCallbacksAndMessages(null);
    // Post the new runnable at the front of the queue
    codecThreadHandler.postAtFrontOfQueue(r);
  }

  public void ensureOnCodecThread(int threadIndex) {
    Handler codecThreadHandler = getHandler(threadIndex);
    if (codecThreadHandler == null || Looper.myLooper() != codecThreadHandler.getLooper()) {
      throw new IllegalStateException("Must be called on codec thread (" + threadIndex
          + "), current: " + Thread.currentThread().getName());
    }
  }

  private synchronized Handler getHandler(int threadIndex) {
    if (mCodecThreadHandlers == null || mCodecThreadHandlers.length == 0) {
      return null;
    }
    if (threadIndex < 0 || threadIndex >= mCodecThreadHandlers.length) {
      threadIndex = DEFAULT_THREAD_INDEX;
    }
    if (mCodecThreadHandlers[threadIndex] == null) {
      String threadName = DEFAULT_THREAD_NAME + "-" + threadIndex;
      HandlerThread codecThread = new HandlerThread(threadName, Process.THREAD_PRIORITY_DISPLAY);
      codecThread.start();
      mCodecThreads[threadIndex] = codecThread;
      mCodecThreadHandlers[threadIndex] = new Handler(codecThread.getLooper());
      AnimaXLog.i(TAG, "Created codec thread handler: " + threadName);
    }
    return mCodecThreadHandlers[threadIndex];
  }

  public int obtainThreadIndex(boolean enableThreadPool) {
    if (!enableThreadPool) {
      return DEFAULT_THREAD_INDEX;
    }
    return (mNextThreadIndex.getAndIncrement() & Integer.MAX_VALUE) % mCodecThreadHandlers.length;
  }
}
