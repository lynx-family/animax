// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.composition;

import androidx.annotation.NonNull;

public interface AnimaXCompositionListener {
  void onCompositionReady(@NonNull AnimaXComposition composition);

  void onCompositionFailed(@NonNull String errorMessage);
}
