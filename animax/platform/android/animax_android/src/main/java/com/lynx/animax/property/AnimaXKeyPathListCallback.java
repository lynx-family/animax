// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import androidx.annotation.NonNull;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.base.bridge.JavaOnlyArray;
import java.util.ArrayList;
import java.util.List;

/**
 * Callback interface for operations that return a list of AnimaXKeyPath objects.
 * Returns an empty list on failure.
 */
public interface AnimaXKeyPathListCallback {
  /**
   * Called when the operation completes (success or failure)
   *
   * @param keyPaths List of AnimaXKeyPath objects returned by the operation.
   *                 Empty list if the operation failed.
   */
  void onCallback(@NonNull List<AnimaXKeyPath> keyPaths);

  /**
   * Method called by native code to report the result of keypath list operations.
   * This is meant for internal use by the AnimaX system.
   *
   * @param keyPathArray JavaOnlyArray containing AnimaXKeyPath objects from native code
   */
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @CalledByNative
  default void onKeyPathListCallback(JavaOnlyArray keyPathArray) {
    if (keyPathArray != null) {
      List<AnimaXKeyPath> keyPaths = new ArrayList<>();
      List<Object> keysList = keyPathArray.asArrayList();
      for (int i = 0; i < keysList.size(); i++) {
        if (keysList.get(i) instanceof JavaOnlyArray) {
          keyPaths.add(new AnimaXKeyPath((JavaOnlyArray) keysList.get(i)));
        }
      }
      onCallback(keyPaths);
    } else {
      onCallback(new ArrayList<>());
    }
  }
}
