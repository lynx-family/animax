// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter.lottie;

import android.graphics.Bitmap;
import androidx.annotation.Nullable;

public interface ImageAssetDelegate {
  @Nullable Bitmap fetchBitmap(LottieImageAsset asset);
}
