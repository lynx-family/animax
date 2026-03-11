// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

/**
 * Enumeration of layer property types that can be animated.
 * These values correspond to the LayerPropertyType enum in the native code.
 */
public enum LayerPropertyType {
  /**
   * The layer's visibility.
   * Type: Integer - when the value is 1, the layer is visible.
   */
  VISIBILITY(1),

  /**
   * The layer's opacity transform.
   * Type: Integer - value is between 0 and 100.
   */
  TRANSFORM_OPACITY(2),

  /**
   * The layer's anchor transform.
   * Type: Point
   */
  TRANSFORM_ANCHOR(3),

  /**
   * The layer's position transform.
   * Type: Point
   */
  TRANSFORM_POSITION(4),

  /**
   * The layer's scale transform.
   * Type: ScaleXY
   */
  TRANSFORM_SCALE(5),

  /**
   * The layer's 2D rotation transform.
   * Type: Float
   */
  TRANSFORM_ROTATION(6),

  /**
   * The layer's skew transform.
   * Type: Float
   */
  TRANSFORM_SKEW(7),

  /**
   * The layer's skew angle transform.
   * Type: Float
   */
  TRANSFORM_SKEW_ANGLE(8),

  /**
   * The layer's start opacity transform.
   * Type: Float
   */
  TRANSFORM_START_OPACITY(9),

  /**
   * The layer's end opacity transform.
   * Type: Float
   */
  TRANSFORM_END_OPACITY(10),

  /**
   * The layer's rotation x transform.
   * Type: Float
   */
  TRANSFORM_ROTATION_X(11),

  /**
   * The layer's 3D rotation y transform.
   * Type: Float
   */
  TRANSFORM_ROTATION_Y(12),

  /**
   * The layer's rotation z transform.
   * Type: Float
   */
  TRANSFORM_ROTATION_Z(13),

  /**
   * The text layer's text value.
   * Type: String
   */
  TEXT_VALUE(101),

  /**
   * The text layer's text size transform.
   * Type: Float
   */
  TEXT_SIZE(102),

  /**
   * The text layer's text color transform.
   * Type: Color
   */
  TEXT_COLOR(103),

  /**
   * The text layer's text tracking transform.
   * Type: Float
   */
  TEXT_TRACKING(104),

  /**
   * The layer or content's color.
   * Type: Color
   */
  COLOR(201),

  /**
   * The layer or content's color filter.
   * Type: ColorFilter
   */
  COLOR_FILTER(202),

  /**
   * The layer or content's stroke color.
   * Type: Color
   */
  STROKE_COLOR(203),

  /**
   * The layer or content's stroke width.
   * Type: Float
   */
  STROKE_WIDTH(204),

  /**
   * The content's opacity.
   * Type: Integer
   */
  OPACITY(205),

  /**
   * The content's blur radius.
   * Type: Float
   */
  BLUR_RADIUS(206),

  /**
   * The content's ellipse size.
   * Type: Point
   */
  ELLIPSE_SIZE(207),

  /**
   * The content's rectangle size.
   * Type: Point
   */
  RECTANGLE_SIZE(208),

  /**
   * The content's corner radius.
   * Type: Float
   */
  CORNER_RADIUS(209),

  /**
   * The content's position.
   * Type: Point
   */
  POSITION(210),

  /**
   * The content's repeater copies.
   * Type: Float
   */
  REPEATER_COPIES(211),

  /**
   * The content's repeater offset.
   * Type: Point
   */
  REPEATER_OFFSET(212),

  /**
   * The content's polygon star points.
   * Type: Float
   */
  POLYSTAR_POINTS(213),

  /**
   * The content's polygon star rotation.
   * Type: Float
   */
  POLYSTAR_ROTATION(214),

  /**
   * The content's polygon star inner radius.
   * Type: Float
   */
  POLYSTAR_INNER_RADIUS(215),

  /**
   * The content's polygon star outer radius.
   * Type: Float
   */
  POLYSTAR_OUTER_RADIUS(216),

  /**
   * The content's polygon star inner rounded.
   * Type: Float
   */
  POLYSTAR_INNER_ROUNDED(217),

  /**
   * The content's polygon star outer rounded.
   * Type: Float
   */
  POLYSTAR_OUTER_ROUNDED(218),

  /**
   * The content's drop shadow color.
   * Type: Color
   */
  DROP_SHADOW_COLOR(219),

  /**
   * The content's drop shadow opacity.
   * Type: Integer
   */
  DROP_SHADOW_OPACITY(220),

  /**
   * The content's drop shadow direction.
   * Type: Float
   */
  DROP_SHADOW_DIRECTION(221),

  /**
   * The content's drop shadow distance.
   * Type: Float
   */
  DROP_SHADOW_DISTANCE(222),

  /**
   * The content's drop shadow radius.
   * Type: Float
   */
  DROP_SHADOW_RADIUS(223);

  private final int mValue;

  LayerPropertyType(int value) {
    this.mValue = value;
  }

  public int getValue() {
    return mValue;
  }
}
