// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;

import android.view.MotionEvent;
import android.view.View;
import androidx.annotation.NonNull;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.IAnimaXCompositionReadyListener;

/**
 * Public interface for AnimaX UI components.
 * Provides all public methods to control an AnimaX view, acting as a facade
 * that delegates most animation logic to an underlying {@link IAnimaXPlayer}.
 * This abstraction allows for multiple parallel implementations with different rendering
 * strategies.
 */
public interface IAnimaXView extends IAnimaXCompositionReadyListener {
  /**
   * Gets the underlying player instance.
   * @return The {@link IAnimaXPlayer} instance.
   */
  @NonNull IAnimaXPlayer getPlayer();

  /**
   * Sets whether the view should ignore its attach/detach status from the window.
   * This allows it to play even when not visible on screen.
   * @param ignore if true, ignores attach status.
   */
  void setIgnoreAttachStatus(boolean ignore);

  /**
   * Enable AnimaXView response the onTapLayers event callback.
   * @param enable if false, the element will not handle touch event.
   */
  void setEnableTapLayerEvent(boolean enable);

  /**
   * Releases all resources associated with this view.
   * After calling this, the instance should not be used again.
   */
  void release();

  /**
   * @see View#requestLayout()
   */
  void requestLayout();

  /**
   * Internal method to handle touch events and delegate them to the player for tap layer detection.
   * @param event The {@link MotionEvent} to handle.
   */
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  default void handleTouchEvent(MotionEvent event) {
    if (event != null && event.getAction() == MotionEvent.ACTION_DOWN) {
      getPlayer().onTap(event.getX(), event.getY());
    }
  }
}
