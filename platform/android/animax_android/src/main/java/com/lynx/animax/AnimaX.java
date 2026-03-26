// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.INativeLibraryLoader;
import com.lynx.animax.util.UIThreadUtils;
import java.lang.reflect.Method;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * AnimaX provides a centralized management for initializing the AnimaX library,
 * loading native dependencies, and setting application context.
 */
public class AnimaX {
  private static final String TAG = "AnimaX";

  private static volatile AnimaX sInstance;

  private volatile boolean mHasLibInit = false;
  private INativeLibraryLoader mNativeLibraryLoader;

  private Context mAppContext;

  private AnimaX() {}

  public static AnimaX inst() {
    if (sInstance == null) {
      synchronized (AnimaX.class) {
        if (sInstance == null) {
          sInstance = new AnimaX();
        }
      }
    }
    return sInstance;
  }

  public void init(Context applicationContext) {
    init(applicationContext, null);
  }

  /**
   * Initializes AnimaX with the default library loader
   */
  @Deprecated
  public void init() {
    init(null, null);
  }

  public void init(Context applicationContext, INativeLibraryLoader loader) {
    setAppContextIfUnset(applicationContext);
    init(loader);
  }

  /**
   * Initializes AnimaX with the optional native library loader.
   * @param loader Optional loader for native libraries.
   */
  @Deprecated
  public void init(INativeLibraryLoader loader) {
    if (!mHasLibInit) {
      initWithLock(loader);
    }
  }

  public Context getAppContext() {
    return mAppContext;
  }

  public float getDensity() {
    float density = 1.f;
    Context context = AnimaX.inst().getAppContext();
    if (context != null) {
      density = context.getResources().getDisplayMetrics().density;
    } else {
      AnimaXLog.e(TAG, "getDensity failed, context is null");
    }
    return density;
  }

  public boolean hasInitialized() {
    return mHasLibInit;
  }

  private synchronized void initWithLock(INativeLibraryLoader loader) {
    if (mHasLibInit) {
      return;
    }

    mNativeLibraryLoader = loader;

    if (!loadLibrary("lynxbase")) {
      AnimaXLog.e(TAG, "AnimaX failed to load liblynxbase.so");
      return;
    }

    if (!loadLibrary("animax")) {
      AnimaXLog.e(TAG, "AnimaX failed to load libanimax.so");
      return;
    }

    // try only, do not fail if textra is not available
    loadLibrary("animax_textra");

    // try only, do not fail if bytevc1 is not available
    loadLibrary("animax_bytevc1");

    nativeRegisterLogger(getALogNativeAddress());

    mHasLibInit = true;
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public boolean loadLibrary(String name) {
    try {
      if (mNativeLibraryLoader != null) {
        mNativeLibraryLoader.loadLibrary(name);
        AnimaXLog.i(TAG, "Native library load " + name + " success with native library loader");
      } else {
        System.loadLibrary(name);
        AnimaXLog.i(TAG, "Native library load " + name + " success with System.loadLibrary");
      }
      return true;
    } catch (UnsatisfiedLinkError error) {
      if (mNativeLibraryLoader == null) {
        AnimaXLog.e(TAG,
            "Native library load " + name + " from system with error message "
                + error.getMessage());
      } else {
        AnimaXLog.e(TAG,
            "Native Library load from " + mNativeLibraryLoader.getClass().getName()
                + " with error message " + error.getMessage());
      }
      return false;
    }
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public void setAppContextIfUnset(Context appContext) {
    if (mAppContext == null && appContext != null) {
      mAppContext = appContext;
      DeviceUtil.initDeviceLowDensity(getDensity());
    }
  }

  /**
   * Get the ALog's SimpleWriteFunc's address by reflection
   * @return native ALog pointer to the ALogFunction
   */
  private long getALogNativeAddress() {
    long address = 0;
    try {
      Class aLogClazz = Class.forName("com.ss.android.agilelogger.ALog");
      Method method = aLogClazz.getMethod("getALogSimpleWriteFuncAddr");
      address = (long) method.invoke(null);
    } catch (Exception e) {
      AnimaXLog.e(TAG, "No ALog found in the host [ " + e.getMessage() + " ]");
    }
    return address;
  }

  private native void nativeRegisterLogger(long loggerPtr);
}
