// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter.lottie;

import android.graphics.Bitmap;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;

public class LottieImageAsset {
  private final int width;
  private final int height;
  private final String id;
  private final String fileName;
  private final String dirName;
  private final boolean hasAlpha;
  /** Pre-set a bitmap for this asset */
  @Nullable private Bitmap bitmap;
  public boolean bitmapHasBeenOptMemory = false;
  public boolean canDownSampleBitmap = false;

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public LottieImageAsset(
      int width, int height, String id, String fileName, String dirName, boolean hasAlpha) {
    this.width = width;
    this.height = height;
    this.id = id;
    this.fileName = fileName;
    this.dirName = dirName;
    this.hasAlpha = hasAlpha;
  }

  public int getWidth() {
    return width;
  }

  public int getHeight() {
    return height;
  }

  public boolean isHasAlpha() {
    return hasAlpha;
  }

  public String getId() {
    return id;
  }

  public String getFileName() {
    return fileName;
  }

  public String getDirName() {
    return dirName;
  }
  /**
   * Returns the bitmap that has been stored for this image asset if one was explicitly set.
   */
  @Nullable
  public Bitmap getBitmap() {
    return bitmap;
  }
  public void setBitmap(@Nullable Bitmap bitmap) {
    this.bitmap = bitmap;
  }
  public boolean hasBitmap() {
    return bitmap != null || (fileName.startsWith("data:") && fileName.indexOf("base64,") > 0);
  }
}
