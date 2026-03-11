// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.base;

import androidx.annotation.RestrictTo;

/**
 * Enum representing different visibility states of a view.
 */
@RestrictTo(RestrictTo.Scope.LIBRARY)
public enum VisibilityState {
  /**
   * Changes when the app enters foreground or background.
   */
  BACKGROUND(1),

  /**
   * Changes when the platform view's setVisibility method is called.
   */
  VISIBLE(1 << 1),

  /**
   * Changes when the platform view's setAlpha method is called.
   */
  OPACITY(1 << 2),

  /**
   * Changes when the platform view's width or height is updated.
   */
  SIZE(1 << 3),

  /**
   * Changes when the platform view is attached to or detached from the window.
   */
  ATTACH(1 << 4),

  /**
   * Represents the aggregated visibility state that combines all other visibility factors.
   * Triggered by onVisibilityAggregated callbacks when visibility changes occur in
   * this view, any of its ancestors, or the window it's attached to. A view is considered
   * truly visible only when it and all its ancestors are visible, attached to the window,
   * and within visible screen bounds.
   */
  AGGREGATED(1 << 5);

  private final int value;

  VisibilityState(int value) {
    this.value = value;
  }

  public int getValue() {
    return value;
  }
}
