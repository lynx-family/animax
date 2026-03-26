// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import androidx.annotation.NonNull;

public class NativePlayerCallback {
  public static void onError(long nativePlayer, @NonNull String errMsg) {
    nativeOnError(nativePlayer, errMsg);
  }

  private static native void nativeOnError(long nativePlayer, String errMsg);
}
