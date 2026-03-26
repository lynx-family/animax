// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.listener;

import androidx.annotation.NonNull;
import java.util.Map;

public class AnimaXErrorParam extends AnimaXParam {
  public static final String KEY_CODE = "code";
  public static final String KEY_MESSAGE = "msg";

  /**
   * Constructor to create an event indicating an error.
   * @param originParams A map containing the original parameters with error details.
   */
  public AnimaXErrorParam(@NonNull Map<String, Object> originParams) {
    super(originParams);
  }

  /**
   * Retrieves the error code associated with the event.
   * @return The error code as an integer.
   */
  public int getErrorCode() {
    return getInt(KEY_CODE);
  }

  /**
   * Retrieves the error message associated with the event.
   * @return The error message as a string.
   */
  public String getErrorMessage() {
    return getString(KEY_MESSAGE);
  }
}
