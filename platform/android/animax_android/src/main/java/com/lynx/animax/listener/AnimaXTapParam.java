// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.listener;

import androidx.annotation.NonNull;
import java.util.List;
import java.util.Map;

public class AnimaXTapParam extends AnimaXParam {
  private static final String KEY_LAYER_LIST = "layerList";

  /**
   * Constructor to create an event containing tap information.
   * @param originParams A map containing the original parameters with event information.
   */
  public AnimaXTapParam(@NonNull Map<String, Object> originParams) {
    super(originParams);
  }

  /**
   * Retrieves the names of the layers hit by the tap event.
   * @return A list of layer names, or an empty list if no layers were hit.
   */
  public List<String> getLayerNames() {
    return getStringArray(KEY_LAYER_LIST);
  }
}
