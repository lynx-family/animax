// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.base;

import androidx.annotation.RestrictTo;

/**
 * Triggered when composition is ready.
 */
@RestrictTo(RestrictTo.Scope.LIBRARY)
public interface IAnimaXCompositionReadyListener {
  void onCompositionReady();
}
