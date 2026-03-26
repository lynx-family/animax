// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CleanupReference;
import com.lynx.animax.service.AnimaXServiceCenter;
import com.lynx.animax.service.IAnimaXResourceFactoryService;
import com.lynx.animax.service.ServiceScope;
import com.lynx.animax.util.AnimaXLog;
import java.util.ArrayList;
import java.util.List;

@RestrictTo(RestrictTo.Scope.LIBRARY)
public class AnimaXResourceLoaderHolder {
  private static final String TAG = "AnimaXResourceLoaderHolder";

  private long mResourceLoaderHolderPtr;

  // This array holds all registered IAnimaXLoader and will be released when the AnimaXPlayer is
  // destroyed.
  private final List<AnimaXLoader> mAnimaXLoaders = new ArrayList<>();

  private CleanupReference mCleanUpReference = null;

  private static class CleanupOnUiThread implements Runnable {
    private long mNativePtr;

    public CleanupOnUiThread(long nativePtr) {
      mNativePtr = nativePtr;
    }

    @Override
    public void run() {
      if (mNativePtr == 0) {
        return;
      }
      AnimaXLog.i(TAG, "Call nativeDestroy.");
      nativeDestroy(mNativePtr);
      mNativePtr = 0;
    }
  }

  public AnimaXResourceLoaderHolder(ServiceScope scope) {
    this(AnimaXServiceCenter.inst().getService(scope, IAnimaXResourceFactoryService.class));
  }

  public AnimaXResourceLoaderHolder(IAnimaXResourceFactoryService resourceFactory) {
    mResourceLoaderHolderPtr = nativeCreate();
    mCleanUpReference =
        new CleanupReference(this, new CleanupOnUiThread(mResourceLoaderHolderPtr), true);

    registerLoaderForScheme(new AnimaXAssetLoader());
    registerLoaderForScheme(new AnimaXFileLoader());
    registerLoaderForScheme(new AnimaXBase64Loader());
    registerLoadersByFactory(resourceFactory);
  }

  private void registerLoadersByFactory(IAnimaXResourceFactoryService resourceFactory) {
    if (resourceFactory == null) {
      AnimaXLog.e(TAG, "resourceFactory is null");
      return;
    }

    List<IAnimaXLoader> loaders = resourceFactory.createAnimaXLoaders();
    if (loaders == null) {
      AnimaXLog.e(TAG, "loaders is null");
      return;
    }

    for (IAnimaXLoader loader : loaders) {
      registerLoaderForScheme(loader);
    }
  }

  public void release() {
    mAnimaXLoaders.clear();
    mCleanUpReference.cleanupNow();
    mResourceLoaderHolderPtr = 0;
  }

  private void registerLoaderForScheme(IAnimaXLoader loader) {
    AnimaXLoader animaXLoader = AnimaXLoader.create(loader);
    mAnimaXLoaders.add(animaXLoader);
    nativeRegisterLoaderForScheme(
        mResourceLoaderHolderPtr, animaXLoader, loader.getScheme().ordinal());
  }

  public long getNativePtr() {
    return mResourceLoaderHolderPtr;
  }

  private static native long nativeCreate();

  private static native void nativeDestroy(long loaderHolder);

  private static native void nativeRegisterLoaderForScheme(
      long loaderHolder, AnimaXLoader loader, int scheme);
}
