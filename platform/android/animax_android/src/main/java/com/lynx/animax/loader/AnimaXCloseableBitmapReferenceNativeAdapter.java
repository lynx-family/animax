// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import android.graphics.Bitmap;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;

@RestrictTo(RestrictTo.Scope.LIBRARY)
public class AnimaXCloseableBitmapReferenceNativeAdapter {
  private final IAnimaXCloseableBitmapReference mCloseableBitmap;

  public static AnimaXCloseableBitmapReferenceNativeAdapter create(
      IAnimaXCloseableBitmapReference closeableBitmapReference) {
    return new AnimaXCloseableBitmapReferenceNativeAdapter(closeableBitmapReference);
  }

  private AnimaXCloseableBitmapReferenceNativeAdapter(
      IAnimaXCloseableBitmapReference closeableBitmapReference) {
    mCloseableBitmap = closeableBitmapReference;
  }

  @CalledByNative
  public Bitmap get() {
    return mCloseableBitmap.get();
  }

  @CalledByNative
  public void close() {
    mCloseableBitmap.close();
  }

  @CalledByNative
  boolean isValid() {
    return mCloseableBitmap.isValid();
  }
}
