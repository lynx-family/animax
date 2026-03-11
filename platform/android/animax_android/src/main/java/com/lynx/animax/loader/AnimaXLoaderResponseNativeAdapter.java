// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import android.graphics.Bitmap;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;

@RestrictTo(RestrictTo.Scope.LIBRARY)
public final class AnimaXLoaderResponseNativeAdapter {
  private final AnimaXLoaderResponse<?> mResponse;

  private AnimaXLoaderResponseNativeAdapter(AnimaXLoaderResponse<?> response) {
    this.mResponse = response;
  }

  public static AnimaXLoaderResponseNativeAdapter create(AnimaXLoaderResponse<?> response) {
    return new AnimaXLoaderResponseNativeAdapter(response);
  }

  @CalledByNative
  public int getType() {
    return mResponse.getType().ordinal();
  }

  @CalledByNative
  public String getErrorMessage() {
    if (mResponse.getType() == AnimaXLoaderResponse.Type.ERROR) {
      Throwable error = (Throwable) mResponse.getData();
      if (error != null) {
        return error.getMessage();
      }
    }
    return "";
  }

  @CalledByNative
  public String getFilePath() {
    return (String) mResponse.getData();
  }

  @CalledByNative
  public byte[] getData() {
    return (byte[]) mResponse.getData();
  }

  @CalledByNative
  public AnimaXCloseableBitmapReferenceNativeAdapter getBitmap() {
    return AnimaXCloseableBitmapReferenceNativeAdapter.create(
        (IAnimaXCloseableBitmapReference) mResponse.getData());
  }
}
