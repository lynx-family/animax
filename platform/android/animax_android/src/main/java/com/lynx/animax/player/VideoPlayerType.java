// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

/**
 * Enumeration defining the type of video player to be used.
 */
public enum VideoPlayerType {
  /**
   * Represents the default video player implementation,
   * which is currently based on Android Media Native APIs.
   */
  DEFAULT,

  /**
   * Represents a custom video player implementation,
   */
  CUSTOM,
}
