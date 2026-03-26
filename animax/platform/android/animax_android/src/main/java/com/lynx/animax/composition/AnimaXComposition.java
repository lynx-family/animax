// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.composition;

import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.base.CleanupReference;
import com.lynx.animax.util.AnimaXLog;

public class AnimaXComposition {
  private static final String TAG = "AnimaXComposition";
  private long mNativePtr;

  private CleanupReference mCleanUpReference = null;

  private static class CleanUpRunnable implements Runnable {
    private long mNativePtr;

    public CleanUpRunnable(long ptr) {
      mNativePtr = ptr;
    }

    @Override
    public void run() {
      AnimaXLog.i(TAG, "nativeDestroy AnimaXComposition on CleanupReference.");
      if (mNativePtr != 0) {
        nativeDestroy(mNativePtr);
        mNativePtr = 0;
      }
    }
  }

  private AnimaXComposition(long nativePtr) {
    mNativePtr = nativePtr;
    mCleanUpReference = new CleanupReference(this, new CleanUpRunnable(mNativePtr), true);
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @CalledByNative
  public static AnimaXComposition create(long nativePtr) {
    return new AnimaXComposition(nativePtr);
  }

  public long getStartFrame() {
    if (!checkNativeReady()) {
      return 0;
    }
    return nativeGetStartFrame(mNativePtr);
  }

  public long getEndFrame() {
    if (!checkNativeReady()) {
      return 0;
    }
    return nativeGetEndFrame(mNativePtr);
  }

  private boolean checkNativeReady() {
    if (mNativePtr == 0) {
      AnimaXLog.e(TAG, "checkNativeReady failed, nativePtr is 0");
      return false;
    }
    return true;
  }

  public long getNativePtr() {
    if (!checkNativeReady()) {
      return 0;
    }
    return mNativePtr;
  }

  public void release() {
    mCleanUpReference.cleanupNow();
    mNativePtr = 0;
  }

  private native long nativeGetStartFrame(long nativePtr);
  private native long nativeGetEndFrame(long nativePtr);
  private static native void nativeDestroy(long nativePtr);
}
