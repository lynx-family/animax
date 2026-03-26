/**
 * Copyright (c) 2015-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.lynx.animax.base.bridge;

import androidx.annotation.Keep;
import java.util.HashMap;

/**
 * Interface for a map that allows typed access to its members. Used to pass parameters from JS to
 * Java.
 */
@Keep
public interface ReadableMap {
  // Not safe as crash will happen while type does not match or key does not exist
  String getString(String name);

  // Get method with default value, this is a safe method
  double getDouble(String name, double defaultValue);
  int getInt(String name, int defaultValue);
  long getLong(String name, long defaultValue);
  ReadableArray getArray(String name);

  ReadableType getType(String name);

  /*
   * Return this reference directly.
   * Please be careful if you are gonna modify it.
   * Besides you can always make a copy of it if necessary.
   */
  HashMap<String, Object> asHashMap();
}
