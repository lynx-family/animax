// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;

/**
 * Enum defining how animation content is aligned within the AnimaXView (container)
 * when the content size and container size do not match.
 * Default: CENTER
 */
public enum ObjectPosition {
  /**
   * "center": (Default) The content is centered both horizontally and vertically
   * within the container.
   */
  CENTER,

  /**
   * "left": The content is aligned to the left edge of the container.
   * Vertical alignment remains centered.
   */
  LEFT,

  /**
   * "right": The content is aligned to the right edge of the container.
   * Vertical alignment remains centered.
   */
  RIGHT,

  /**
   * "top": The content is aligned to the top edge of the container.
   * Horizontal alignment remains centered.
   */
  TOP,

  /**
   * "bottom": The content is aligned to the bottom edge of the container.
   * Horizontal alignment remains centered.
   */
  BOTTOM;
}
