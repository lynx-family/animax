// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter.lottie.model;

import java.util.Arrays;
import java.util.List;

public class KeyPath {
  public List<String> keys;

  public KeyPath(String... keys) {
    this.keys = Arrays.asList(keys);
  }

  public String[] getKeysArray() {
    return keys.toArray(new String[0]);
  }

  @Override
  public String toString() {
    return "KeyPath{"
        + "keys=" + keys + '}';
  }
}
