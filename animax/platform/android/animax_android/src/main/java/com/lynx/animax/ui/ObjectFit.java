// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;

/**
 * Enum defining how animation content fits into AnimaXView (container) size
 * Default: CONTAIN
 */
public enum ObjectFit {
  /**
   * "center": The content is kept at its original size and centered within the container.
   * If the content's size is larger than the container size, it will overflow from all sides
   * evenly.
   */
  CENTER,

  /**
   * "cover": The content is scaled to completely cover the container while maintaining its aspect
   * ratio. If the actual content's aspect ratio is different from the container, the content will
   * be clipped to fill the container.
   */
  COVER,

  /**
   * "contain": (Default) The content is scaled to fit within the container, ensuring all content is
   * visible, while maintaining its aspect ratio. If the content's aspect ratio is different from
   * the container, the remaining space of the container will show the container's background.
   */
  CONTAIN,

  /**
   * "fill": The content is scaled to fill the entire container, regardless of its aspect
   * ratio. This may cause the content to be stretched or squished to fit the container's
   * dimensions.
   */
  FILL,

  /**
   * The content is sized as if using "center" or "contain" whichever results in a smaller
   * rendered size.
   */
  SCALE_DOWN
}
