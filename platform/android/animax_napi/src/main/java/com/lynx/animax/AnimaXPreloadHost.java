// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax;

import androidx.annotation.Keep;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.base.CleanupReference;
import com.lynx.animax.loader.AnimaXResourceLoaderHolder;
import com.lynx.animax.util.AnimaXLog;
import com.lynx.animax.util.UIThreadUtils;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

@RestrictTo(RestrictTo.Scope.LIBRARY)
@Keep
public final class AnimaXPreloadHost {
  public interface Provider {
    AnimaXResourceLoaderHolder getResourceLoader();
    float getDeviceDensity();
  }

  private final WeakReference<Provider> mProvider;
  private long mNativePtr;
  // Releases the native host once, through invalidate() or after this object
  // is collected without it.
  private final CleanupReference mNativeCleanup;
  // Accessed only by the owning runtime's attach/detach callbacks.
  private long mEnv;
  // Protected by this host's lock; only outstanding requests are retained.
  private List<Long> mPendingCapabilities = new ArrayList<>();

  public AnimaXPreloadHost(Provider provider) {
    mProvider = new WeakReference<>(provider);
    mNativePtr = nativeCreate();
    // Capture only the pointer so that this object can still be collected.
    final long nativePtr = mNativePtr;
    mNativeCleanup = new CleanupReference(this, () -> nativeDestroy(nativePtr), false);
  }

  public synchronized boolean onRuntimeAttach(long env) {
    if (mNativePtr == 0 || env == 0)
      return false;
    if (mEnv == env)
      return true;
    if (mEnv == 0 && nativeAttach(mNativePtr, env)) {
      mEnv = env;
      return true;
    }
    return false;
  }

  public synchronized void onRuntimeDetach() {
    if (mEnv != 0) {
      nativeDetach(mEnv);
      mEnv = 0;
    }
    invalidate();
  }

  public synchronized void invalidate() {
    if (mNativePtr != 0) {
      mNativePtr = 0;
      // Runs nativeDestroy on this thread, as before.
      mNativeCleanup.cleanupNow();
    }
  }

  public synchronized boolean attachPlayer(AnimaXPlayer player) {
    UIThreadUtils.assertOnUiThread();
    return mNativePtr != 0 && nativeAttachPlayer(mNativePtr, player);
  }

  @CalledByNative
  private static long getPlayerPointer(AnimaXPlayer player) {
    return player.getNativePtrForPreload();
  }

  @CalledByNative
  private synchronized boolean requestCapabilities(long callback) {
    boolean dispatch = mPendingCapabilities.isEmpty();
    mPendingCapabilities.add(callback);
    if (dispatch && !UIThreadUtils.getUiThreadHandler().post(this::completeCapabilities)) {
      // false leaves ownership with C++; no other caller can join this batch
      // until the dispatch result is known and this pointer has been removed.
      mPendingCapabilities.remove(mPendingCapabilities.size() - 1);
      return false;
    }
    return true;
  }

  private void completeCapabilities() {
    List<Long> callbacks;
    boolean valid;
    synchronized (this) {
      callbacks = mPendingCapabilities;
      mPendingCapabilities = new ArrayList<>();
      valid = mNativePtr != 0;
    }
    // New requests form a new batch. Do not retain a scale snapshot between
    // batches or call the page provider while holding the host lock.
    AnimaXResourceLoaderHolder loader = null;
    float density = 0;
    try {
      Provider provider = mProvider.get();
      if (valid && provider != null) {
        density = provider.getDeviceDensity();
        if (!Float.isNaN(density) && !Float.isInfinite(density) && density > 0) {
          loader = provider.getResourceLoader();
        }
      }
    } catch (RuntimeException e) {
      AnimaXLog.e("AnimaXPreloadHost", "Failed to create resource services: " + e);
    } finally {
      // Accepted callbacks are consumed even after invalidation or provider
      // failure. Holder access and JNI handoff stay on UI, as does its release.
      for (long callback : callbacks) {
        nativeCompleteCapabilities(callback, loader == null ? 0 : loader.getNativePtr(), density);
      }
    }
  }

  private native long nativeCreate();
  private static native void nativeDestroy(long host);
  private static native boolean nativeAttach(long host, long napiEnv);
  private static native void nativeDetach(long napiEnv);
  private static native boolean nativeAttachPlayer(long host, AnimaXPlayer player);
  private static native void nativeCompleteCapabilities(long callback, long loader, float density);
}
