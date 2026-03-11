// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import android.graphics.Bitmap;
import androidx.annotation.NonNull;

public final class AnimaXLoaderResponse<T> {
  public enum Type { STRING_FILE_PATH, BYTE_ARRAY, BITMAP, ERROR }
  private final Type mType;
  private final T mData;

  public static AnimaXLoaderResponse<String> createStringFilePathResponse(
      @NonNull String filePath) {
    return new AnimaXLoaderResponse<>(Type.STRING_FILE_PATH, filePath);
  }

  public static AnimaXLoaderResponse<byte[]> createByteArrayResponse(@NonNull byte[] bytes) {
    return new AnimaXLoaderResponse<>(Type.BYTE_ARRAY, bytes);
  }

  public static AnimaXLoaderResponse<IAnimaXCloseableBitmapReference> createBitmapResponse(
      @NonNull IAnimaXCloseableBitmapReference bitmap) {
    return new AnimaXLoaderResponse<>(Type.BITMAP, bitmap);
  }

  public static AnimaXLoaderResponse<Throwable> createErrorResponse(@NonNull Throwable error) {
    return new AnimaXLoaderResponse<>(Type.ERROR, error);
  }

  public T getData() {
    return mData;
  }

  Type getType() {
    return mType;
  }

  private AnimaXLoaderResponse(Type type, T data) {
    this.mType = type;
    this.mData = data;
  }
}
