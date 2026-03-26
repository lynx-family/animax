// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import junit.framework.TestCase;
import org.junit.Test;

public class PropertyTypeTest extends TestCase {
  @Test
  public void testLayerPropertyTypeValues() {
    assertEquals(1, LayerPropertyType.VISIBILITY.getValue());
    assertEquals(2, LayerPropertyType.TRANSFORM_OPACITY.getValue());
    assertEquals(3, LayerPropertyType.TRANSFORM_ANCHOR.getValue());
    assertEquals(4, LayerPropertyType.TRANSFORM_POSITION.getValue());
    assertEquals(5, LayerPropertyType.TRANSFORM_SCALE.getValue());
    assertEquals(6, LayerPropertyType.TRANSFORM_ROTATION.getValue());
    assertEquals(7, LayerPropertyType.TRANSFORM_SKEW.getValue());
    assertEquals(8, LayerPropertyType.TRANSFORM_SKEW_ANGLE.getValue());
    assertEquals(9, LayerPropertyType.TRANSFORM_START_OPACITY.getValue());
    assertEquals(10, LayerPropertyType.TRANSFORM_END_OPACITY.getValue());
    assertEquals(11, LayerPropertyType.TRANSFORM_ROTATION_X.getValue());
    assertEquals(12, LayerPropertyType.TRANSFORM_ROTATION_Y.getValue());
    assertEquals(13, LayerPropertyType.TRANSFORM_ROTATION_Z.getValue());
    assertEquals(101, LayerPropertyType.TEXT_VALUE.getValue());
    assertEquals(102, LayerPropertyType.TEXT_SIZE.getValue());
    assertEquals(103, LayerPropertyType.TEXT_COLOR.getValue());
    assertEquals(104, LayerPropertyType.TEXT_TRACKING.getValue());
    assertEquals(201, LayerPropertyType.COLOR.getValue());
    assertEquals(202, LayerPropertyType.COLOR_FILTER.getValue());
    assertEquals(203, LayerPropertyType.STROKE_COLOR.getValue());
    assertEquals(204, LayerPropertyType.STROKE_WIDTH.getValue());
    assertEquals(205, LayerPropertyType.OPACITY.getValue());
    assertEquals(206, LayerPropertyType.BLUR_RADIUS.getValue());
    assertEquals(207, LayerPropertyType.ELLIPSE_SIZE.getValue());
    assertEquals(208, LayerPropertyType.RECTANGLE_SIZE.getValue());
    assertEquals(209, LayerPropertyType.CORNER_RADIUS.getValue());
    assertEquals(210, LayerPropertyType.POSITION.getValue());
    assertEquals(211, LayerPropertyType.REPEATER_COPIES.getValue());
    assertEquals(212, LayerPropertyType.REPEATER_OFFSET.getValue());
    assertEquals(213, LayerPropertyType.POLYSTAR_POINTS.getValue());
    assertEquals(214, LayerPropertyType.POLYSTAR_ROTATION.getValue());
    assertEquals(215, LayerPropertyType.POLYSTAR_INNER_RADIUS.getValue());
    assertEquals(216, LayerPropertyType.POLYSTAR_OUTER_RADIUS.getValue());
    assertEquals(217, LayerPropertyType.POLYSTAR_INNER_ROUNDED.getValue());
    assertEquals(218, LayerPropertyType.POLYSTAR_OUTER_ROUNDED.getValue());
    assertEquals(219, LayerPropertyType.DROP_SHADOW_COLOR.getValue());
    assertEquals(220, LayerPropertyType.DROP_SHADOW_OPACITY.getValue());
    assertEquals(221, LayerPropertyType.DROP_SHADOW_DIRECTION.getValue());
    assertEquals(222, LayerPropertyType.DROP_SHADOW_DISTANCE.getValue());
    assertEquals(223, LayerPropertyType.DROP_SHADOW_RADIUS.getValue());
  }

  @Test
  public void testResourcePropertyTypeValues() {
    assertEquals(1, ResourcePropertyType.IMAGE_DIR_NAME.getValue());
    assertEquals(2, ResourcePropertyType.IMAGE_FILE_NAME.getValue());
    assertEquals(3, ResourcePropertyType.IMAGE_WIDTH.getValue());
    assertEquals(4, ResourcePropertyType.IMAGE_HEIGHT.getValue());
    assertEquals(101, ResourcePropertyType.FONT_FAMILY.getValue());
    assertEquals(102, ResourcePropertyType.FONT_STYLE.getValue());
    assertEquals(103, ResourcePropertyType.FONT_ASCENT.getValue());
    assertEquals(104, ResourcePropertyType.FONT_PATH.getValue());
    assertEquals(201, ResourcePropertyType.VIDEO_DIR_NAME.getValue());
    assertEquals(202, ResourcePropertyType.VIDEO_FILE_NAME.getValue());
    assertEquals(203, ResourcePropertyType.VIDEO_WIDTH.getValue());
    assertEquals(204, ResourcePropertyType.VIDEO_HEIGHT.getValue());
  }
}
