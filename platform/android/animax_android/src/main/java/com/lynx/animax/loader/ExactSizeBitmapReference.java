// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import android.graphics.Bitmap;
import androidx.annotation.NonNull;
import com.lynx.animax.util.AnimaXLog;

/**
 * Image services may ignore the requested size, so scale their bitmap when it is consumed. Native
 * code reads the reference on the resource thread, which keeps this work off the service's callback
 * thread.
 */
final class ExactSizeBitmapReference implements IAnimaXCloseableBitmapReference {
  private static final String TAG = "ExactSizeBitmapReference";
  private static final int MAX_BYTES_PER_PIXEL = 8;

  private final IAnimaXCloseableBitmapReference mSource;
  private final int mWidth;
  private final int mHeight;
  private Bitmap mScaledBitmap;

  ExactSizeBitmapReference(@NonNull IAnimaXCloseableBitmapReference source, int width, int height) {
    mSource = source;
    mWidth = width;
    mHeight = height;
  }

  @Override
  public Bitmap get() {
    if (mScaledBitmap != null) {
      return mScaledBitmap;
    }
    Bitmap source = mSource.get();
    if (source == null || (source.getWidth() == mWidth && source.getHeight() == mHeight)) {
      return source;
    }
    // Android can abort instead of throwing when a bitmap allocation overflows.
    if ((long) mWidth * mHeight > Integer.MAX_VALUE / MAX_BYTES_PER_PIXEL) {
      AnimaXLog.e(TAG, "Bitmap size is out of range: " + mWidth + "x" + mHeight);
      return null;
    }
    try {
      mScaledBitmap = Bitmap.createScaledBitmap(source, mWidth, mHeight, true);
      return mScaledBitmap;
    } catch (RuntimeException | OutOfMemoryError e) {
      AnimaXLog.e(TAG, "Failed to scale bitmap: " + e.getMessage());
      return null;
    }
  }

  @Override
  public void close() {
    if (mScaledBitmap != null) {
      mScaledBitmap.recycle();
      mScaledBitmap = null;
    }
    mSource.close();
  }

  @Override
  public boolean isValid() {
    return mSource.isValid();
  }
}
