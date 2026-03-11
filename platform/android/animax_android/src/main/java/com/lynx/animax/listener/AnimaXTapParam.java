// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.listener;

import androidx.annotation.NonNull;
import java.util.Map;

public class AnimaXTapParam extends AnimaXParam {
  /**
   * Constructor to create an event containing tap information.
   * @param originParams A map containing the original parameters with event information.
   */
  public AnimaXTapParam(@NonNull Map<String, Object> originParams) {
    super(originParams);
  }

  // todo: provide getLayerNames() after JNI standalone is complete.
}
