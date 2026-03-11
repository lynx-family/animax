// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import android.graphics.Bitmap;
import androidx.annotation.NonNull;

public class SimpleCloseableBitmapReference implements IAnimaXCloseableBitmapReference {
  private final Bitmap mBitmap;

  public SimpleCloseableBitmapReference(@NonNull Bitmap bitmap) {
    mBitmap = bitmap;
  }

  @Override
  public Bitmap get() {
    if (isValid()) {
      return mBitmap;
    } else {
      return null;
    }
  }

  @Override
  public void close() {
    mBitmap.recycle();
  }

  @Override
  public boolean isValid() {
    return !mBitmap.isRecycled();
  }
}
