// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;
import java.util.concurrent.atomic.AtomicBoolean;

@RestrictTo(RestrictTo.Scope.LIBRARY)
public class AnimaXLoaderCompletionHandler implements IAnimaXLoaderCompletionHandler {
  private final long mNativePtr;
  private final AtomicBoolean mCompleted = new AtomicBoolean(false);
  public AnimaXLoaderCompletionHandler(long nativePtr) {
    this.mNativePtr = nativePtr;
  }

  @CalledByNative
  static public AnimaXLoaderCompletionHandler createCompletionHandler(long nativePtr) {
    return new AnimaXLoaderCompletionHandler(nativePtr);
  }

  @Override
  public void onComplete(AnimaXLoaderResponse<?> response) {
    if (!mCompleted.getAndSet(true)) {
      nativeOnComplete(this.mNativePtr, AnimaXLoaderResponseNativeAdapter.create(response));
    }
  }

  public native void nativeOnComplete(
      long nativePtr, AnimaXLoaderResponseNativeAdapter loaderResponse);
}
