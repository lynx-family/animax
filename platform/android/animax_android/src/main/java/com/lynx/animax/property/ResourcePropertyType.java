// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

/**
 * Enumeration of resource property types that can be modified.
 */
public enum ResourcePropertyType {
  /**
   * Directory name for image resources
   */
  IMAGE_DIR_NAME(1),

  /**
   * File name for image resources
   */
  IMAGE_FILE_NAME(2),

  /**
   * Width of image resources
   */
  IMAGE_WIDTH(3),

  /**
   * Height of image resources
   */
  IMAGE_HEIGHT(4),

  /**
   * Font family for text elements
   */
  FONT_FAMILY(101),

  /**
   * Font style for text elements
   */
  FONT_STYLE(102),

  /**
   * Font ascent for text elements
   */
  FONT_ASCENT(103),

  /**
   * Font file path for text elements
   */
  FONT_PATH(104),

  /**
   * Directory name for video resources
   */
  VIDEO_DIR_NAME(201),

  /**
   * File name for video resources
   */
  VIDEO_FILE_NAME(202),

  /**
   * Width of video resources
   */
  VIDEO_WIDTH(203),

  /**
   * Height of video resources
   */
  VIDEO_HEIGHT(204);

  private final int mValue;

  ResourcePropertyType(int value) {
    this.mValue = value;
  }

  public int getValue() {
    return mValue;
  }
}
