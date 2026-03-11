// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.base.bridge.JavaOnlyArray;
import java.util.ArrayList;
import java.util.List;

/**
 * Callback interface for property update operations.
 * Provides separate methods for success and error cases.
 */
public interface AnimaXPropertyCallback {
  /**
   * Called when a property update operation completes successfully
   */
  void onSuccess();

  /**
   * Called when a property update operation fails
   *
   * @param errorMessage Error message indicating the reason for failure
   */
  void onError(@NonNull List<String> errorMessageList);

  /**
   * Method called by native code to report the result of property operations.
   * This is meant for internal use by the AnimaX system.
   *
   * @param success Whether the operation succeeded
   * @param errorMessage Error message in case of failure
   */
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @CalledByNative
  default void onPropertyCallback(JavaOnlyArray errorMessages) {
    if (errorMessages == null || errorMessages.isEmpty()) {
      onSuccess();
      return;
    }

    List<String> errorMessageList = new ArrayList<>();
    List<Object> errorMessageObjectList = errorMessages.asArrayList();
    for (int i = 0; i < errorMessageObjectList.size(); i++) {
      Object errorMessageObject = errorMessageObjectList.get(i);
      if (errorMessageObject instanceof String) {
        errorMessageList.add((String) errorMessageObject);
      }
    }
    onError(errorMessageList);
  }
}
