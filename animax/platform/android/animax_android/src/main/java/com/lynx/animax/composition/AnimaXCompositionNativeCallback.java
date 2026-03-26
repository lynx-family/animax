// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.composition;

import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;

@RestrictTo(RestrictTo.Scope.LIBRARY)
public class AnimaXCompositionNativeCallback {
  private final AnimaXCompositionListener mListener;

  public AnimaXCompositionNativeCallback(AnimaXCompositionListener listener) {
    mListener = listener;
  }

  @CalledByNative
  void onCompositionCallback(AnimaXComposition composition, String errorMessage) {
    if (mListener == null) {
      return;
    }

    if (composition != null) {
      mListener.onCompositionReady(composition);
    } else if (errorMessage != null) {
      mListener.onCompositionFailed(errorMessage);
    } else {
      mListener.onCompositionFailed("Unknown error");
    }
  }
}
