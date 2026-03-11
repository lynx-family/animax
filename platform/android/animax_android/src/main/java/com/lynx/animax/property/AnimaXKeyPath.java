// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import androidx.annotation.NonNull;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.base.bridge.JavaOnlyArray;
import com.lynx.animax.util.AnimaXLog;
import java.util.ArrayList;
import java.util.List;

/**
 * A path that can target specific layers or elements in an AnimaX animation.
 *
 * KeyPath supports wildcards ('*') to match exactly one item and globstars ('**')
 * to match zero or more items, enabling flexible targeting of animation elements.
 * Null or empty keys provided in the constructor will be ignored.
 *
 * Example hierarchy:
 * MainScene (Layer)
 *     Background (Layer)
 *         Sky (Layer)
 *             Fill (Property)
 *         Mountains (Layer)
 *             Fill (Property)
 *     Character (Layer)
 *         Body (Layer)
 *             Fill (Property)
 *         Face (Layer)
 *             Eyes (Layer)
 *                 Fill (Property)
 *
 * Example usage:
 * - Target Character's Body Fill:
 *   new AnimaXKeyPath("Character", "Body", "Fill")
 * - Target all Fills in Character:
 *   new AnimaXKeyPath("Character", "**", "Fill")
 * - Target all Fills in the animation:
 *   new AnimaXKeyPath("**", "Fill")
 */
public class AnimaXKeyPath {
  private static final String TAG = "AnimaXKeyPath";
  private final JavaOnlyArray mKeys;

  /**
   * Creates a KeyPath with the specified key segments
   *
   * @param keys The key segments that define the path
   */
  public AnimaXKeyPath(@NonNull String... keys) {
    this.mKeys = new JavaOnlyArray();
    for (String key : keys) {
      if (key == null || key.isEmpty()) {
        AnimaXLog.i(TAG, "AnimaXKeyPath received a null or empty key, which will be ignored.");
        continue;
      }
      this.mKeys.pushString(key);
    }
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public AnimaXKeyPath(@NonNull JavaOnlyArray keys) {
    this.mKeys = keys;
  }

  /**
   * Gets all keys as a JavaOnlyArray
   *
   * @return JavaOnlyArray of all keys
   */
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @CalledByNative
  public JavaOnlyArray getKeys() {
    return mKeys;
  }

  public List<String> getKeysList() {
    List<String> keyList = new ArrayList<>();
    for (Object item : mKeys) {
      if (item instanceof String) {
        keyList.add((String) item);
      }
    }
    return keyList;
  }

  @NonNull
  @Override
  public String toString() {
    return "KeyPath{keys=" + getKeysList().toString() + "}";
  }
}
