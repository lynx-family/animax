// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter.lottie.value;

import androidx.annotation.Nullable;

public class LottieValueCallback<T> {
  @Nullable protected T value = null;

  public LottieValueCallback() {}

  public LottieValueCallback(@Nullable T staticValue) {
    value = staticValue;
  }

  @Nullable
  public T getValue(LottieFrameInfo<T> frameInfo) {
    return value;
  }
}
