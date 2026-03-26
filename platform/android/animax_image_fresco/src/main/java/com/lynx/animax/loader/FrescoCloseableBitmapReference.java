// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import android.graphics.Bitmap;
import androidx.annotation.NonNull;
import com.facebook.common.references.CloseableReference;
import com.lynx.animax.loader.IAnimaXCloseableBitmapReference;

public class FrescoCloseableBitmapReference implements IAnimaXCloseableBitmapReference {
  private final CloseableReference<Bitmap> mBitmapCloseableReference;

  public FrescoCloseableBitmapReference(
      @NonNull CloseableReference<Bitmap> bitmapCloseableReference) {
    mBitmapCloseableReference = bitmapCloseableReference.clone();
  }

  @Override
  public Bitmap get() {
    if (mBitmapCloseableReference.isValid()) {
      return mBitmapCloseableReference.get();
    } else {
      return null;
    }
  }

  @Override
  public void close() {
    mBitmapCloseableReference.close();
  }

  @Override
  public boolean isValid() {
    return mBitmapCloseableReference.isValid();
  }
}
