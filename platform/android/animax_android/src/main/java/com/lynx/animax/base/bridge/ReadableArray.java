// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.base.bridge;

/**
 * Interface for an array that allows typed access to its members.
 */
public interface ReadableArray {
  int size();
  String getString(int index);
}
