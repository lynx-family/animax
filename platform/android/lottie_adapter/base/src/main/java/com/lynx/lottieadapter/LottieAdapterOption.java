// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter;

import androidx.annotation.NonNull;

public class LottieAdapterOption {
  private AnimationType mType = AnimationType.LOTTIE;
  private boolean mUseImageMode = false;
  private boolean mMultiThreadAccelerate = false;

  public LottieAdapterOption() {}

  public LottieAdapterOption(@NonNull LottieAdapterOption option) {
    copyFromOption(option);
  }

  public AnimationType getType() {
    return mType;
  }

  public void setType(AnimationType type) {
    mType = type;
  }

  public boolean isUseImageMode() {
    return mUseImageMode;
  }

  public void setUseImageMode(boolean useImageMode) {
    mUseImageMode = useImageMode;
  }

  public boolean isMultiThreadAccelerate() {
    return mMultiThreadAccelerate;
  }

  public void setMultiThreadAccelerate(boolean multiThreadAccelerate) {
    mMultiThreadAccelerate = multiThreadAccelerate;
  }

  public void copyFromOption(@NonNull LottieAdapterOption option) {
    mType = option.mType;
    mUseImageMode = option.mUseImageMode;
    mMultiThreadAccelerate = option.mMultiThreadAccelerate;
  }
}
