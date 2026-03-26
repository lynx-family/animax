// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import junit.framework.TestCase;
import org.junit.Test;

public class AnimaXValueParamTest extends TestCase {
  @Test
  public void testFromString() {
    String testValue = "test_string";
    AnimaXValueParam param = AnimaXValueParam.fromString(testValue);

    assertTrue(param.isString());
    assertFalse(param.isNumber());
    assertFalse(param.isBoolean());
    assertFalse(param.isCoordinate());
    assertFalse(param.isColor());
    assertFalse(param.isColorFilter());
    assertFalse(param.isNull());

    assertEquals(testValue, param.getStringValue());
    assertFalse(param.hasTargetFrame());
    assertEquals(-1, param.getTargetFrame());
  }

  @Test
  public void testFromStringWithTargetFrame() {
    String testValue = "test_string";
    int targetFrame = 10;
    AnimaXValueParam param = AnimaXValueParam.fromString(testValue, targetFrame);

    assertTrue(param.isString());
    assertEquals(testValue, param.getStringValue());
    assertTrue(param.hasTargetFrame());
    assertEquals(targetFrame, param.getTargetFrame());
  }

  @Test
  public void testFromStringEmpty() {
    AnimaXValueParam param = AnimaXValueParam.fromString("");
    assertTrue(param.isString());
    assertEquals("", param.getStringValue());
  }

  @Test
  public void testFromStringNull() {
    AnimaXValueParam param = AnimaXValueParam.fromString(null);
    assertTrue(param.isString());
    assertNull(param.getStringValue());
  }

  @Test
  public void testFromNumber() {
    double testValue = 42.5;
    AnimaXValueParam param = AnimaXValueParam.fromNumber(testValue);

    assertFalse(param.isString());
    assertTrue(param.isNumber());
    assertFalse(param.isBoolean());
    assertFalse(param.isCoordinate());
    assertFalse(param.isColor());
    assertFalse(param.isColorFilter());
    assertFalse(param.isNull());

    assertEquals(testValue, param.getNumberValue(), 0.001);
    assertFalse(param.hasTargetFrame());
  }

  @Test
  public void testFromNumberWithTargetFrame() {
    double testValue = 100.0;
    int targetFrame = 5;
    AnimaXValueParam param = AnimaXValueParam.fromNumber(testValue, targetFrame);

    assertTrue(param.isNumber());
    assertEquals(testValue, param.getNumberValue(), 0.001);
    assertTrue(param.hasTargetFrame());
    assertEquals(targetFrame, param.getTargetFrame());
  }

  @Test
  public void testFromNumberZero() {
    AnimaXValueParam param = AnimaXValueParam.fromNumber(0.0);
    assertTrue(param.isNumber());
    assertEquals(0.0, param.getNumberValue(), 0.001);
  }

  @Test
  public void testFromNumberNegative() {
    double testValue = -15.5;
    AnimaXValueParam param = AnimaXValueParam.fromNumber(testValue);
    assertEquals(testValue, param.getNumberValue(), 0.001);
  }

  @Test
  public void testFromBooleanTrue() {
    AnimaXValueParam param = AnimaXValueParam.fromBoolean(true);

    assertFalse(param.isString());
    assertFalse(param.isNumber());
    assertTrue(param.isBoolean());
    assertFalse(param.isCoordinate());
    assertFalse(param.isColor());
    assertFalse(param.isColorFilter());
    assertFalse(param.isNull());

    assertTrue(param.getBooleanValue());
    assertFalse(param.hasTargetFrame());
  }

  @Test
  public void testFromBooleanFalse() {
    AnimaXValueParam param = AnimaXValueParam.fromBoolean(false);
    assertTrue(param.isBoolean());
    assertFalse(param.getBooleanValue());
  }

  @Test
  public void testFromBooleanWithTargetFrame() {
    boolean testValue = true;
    int targetFrame = 15;
    AnimaXValueParam param = AnimaXValueParam.fromBoolean(testValue, targetFrame);

    assertTrue(param.isBoolean());
    assertEquals(testValue, param.getBooleanValue());
    assertTrue(param.hasTargetFrame());
    assertEquals(targetFrame, param.getTargetFrame());
  }

  @Test
  public void testFromCoordinate2D() {
    double x = 10.5;
    double y = 20.7;
    AnimaXValueParam param = AnimaXValueParam.fromCoordinate(x, y);

    assertFalse(param.isString());
    assertFalse(param.isNumber());
    assertFalse(param.isBoolean());
    assertTrue(param.isCoordinate());
    assertFalse(param.isColor());
    assertFalse(param.isColorFilter());
    assertFalse(param.isNull());

    assertEquals(x, param.getX(), 0.001);
    assertEquals(y, param.getY(), 0.001);
    assertEquals(0.0, param.getZ(), 0.001);
    assertFalse(param.hasTargetFrame());
  }

  @Test
  public void testFromCoordinate2DWithTargetFrame() {
    double x = 100.0;
    double y = 200.0;
    int targetFrame = 25;
    AnimaXValueParam param = AnimaXValueParam.fromCoordinate(x, y, targetFrame);

    assertTrue(param.isCoordinate());
    assertEquals(x, param.getX(), 0.001);
    assertEquals(y, param.getY(), 0.001);
    assertEquals(0.0, param.getZ(), 0.001);
    assertTrue(param.hasTargetFrame());
    assertEquals(targetFrame, param.getTargetFrame());
  }

  @Test
  public void testFromCoordinate3D() {
    double x = 10.5;
    double y = 20.7;
    double z = 30.2;
    AnimaXValueParam param = AnimaXValueParam.fromCoordinate(x, y, z);

    assertTrue(param.isCoordinate());
    assertEquals(x, param.getX(), 0.001);
    assertEquals(y, param.getY(), 0.001);
    assertEquals(z, param.getZ(), 0.001);
    assertFalse(param.hasTargetFrame());
  }

  @Test
  public void testFromCoordinate3DWithTargetFrame() {
    double x = 100.0;
    double y = 200.0;
    double z = 300.0;
    int targetFrame = 25;
    AnimaXValueParam param = AnimaXValueParam.fromCoordinate(x, y, z, targetFrame);

    assertTrue(param.isCoordinate());
    assertEquals(x, param.getX(), 0.001);
    assertEquals(y, param.getY(), 0.001);
    assertEquals(z, param.getZ(), 0.001);
    assertTrue(param.hasTargetFrame());
    assertEquals(targetFrame, param.getTargetFrame());
  }

  @Test
  public void testFromColor() {
    int color = 0xFFFF0000;
    AnimaXValueParam param = AnimaXValueParam.fromColor(color);

    assertFalse(param.isString());
    assertFalse(param.isNumber());
    assertFalse(param.isBoolean());
    assertFalse(param.isCoordinate());
    assertTrue(param.isColor());
    assertFalse(param.isColorFilter());
    assertFalse(param.isNull());

    assertEquals(color, param.getColorValue());
    assertFalse(param.hasTargetFrame());
  }

  @Test
  public void testFromColorWithTargetFrame() {
    int color = 0xFF00FF00;
    int targetFrame = 30;
    AnimaXValueParam param = AnimaXValueParam.fromColor(color, targetFrame);

    assertTrue(param.isColor());
    assertEquals(color, param.getColorValue());
    assertTrue(param.hasTargetFrame());
    assertEquals(targetFrame, param.getTargetFrame());
  }

  @Test
  public void testFromColorTransparent() {
    int color = 0x00000000;
    AnimaXValueParam param = AnimaXValueParam.fromColor(color);
    assertEquals(color, param.getColorValue());
  }

  @Test
  public void testFromColorFilter() {
    int color = 0xFF0000FF;
    int mode = 3;
    AnimaXValueParam param = AnimaXValueParam.fromColorFilter(color, mode);

    assertFalse(param.isString());
    assertFalse(param.isNumber());
    assertFalse(param.isBoolean());
    assertFalse(param.isCoordinate());
    assertFalse(param.isColor());
    assertTrue(param.isColorFilter());
    assertFalse(param.isNull());

    assertEquals(color, param.getColorValue());
    assertEquals(mode, param.getFilterMode());
    assertFalse(param.hasTargetFrame());
  }

  @Test
  public void testFromColorFilterWithTargetFrame() {
    int color = 0xFFFFFF00;
    int mode = 5;
    int targetFrame = 40;
    AnimaXValueParam param = AnimaXValueParam.fromColorFilter(color, mode, targetFrame);

    assertTrue(param.isColorFilter());
    assertEquals(color, param.getColorValue());
    assertEquals(mode, param.getFilterMode());
    assertTrue(param.hasTargetFrame());
    assertEquals(targetFrame, param.getTargetFrame());
  }

  @Test
  public void testCreateNull() {
    AnimaXValueParam param = AnimaXValueParam.createNull();

    assertFalse(param.isString());
    assertFalse(param.isNumber());
    assertFalse(param.isBoolean());
    assertFalse(param.isCoordinate());
    assertFalse(param.isColor());
    assertFalse(param.isColorFilter());
    assertTrue(param.isNull());

    assertFalse(param.hasTargetFrame());
    assertEquals(-1, param.getTargetFrame());
  }

  @Test
  public void testDefaultValuesForWrongType() {
    AnimaXValueParam stringParam = AnimaXValueParam.fromString("test");

    assertEquals(0.0, stringParam.getNumberValue(), 0.001);
    assertFalse(stringParam.getBooleanValue());
    assertEquals(0.0, stringParam.getX(), 0.001);
    assertEquals(0.0, stringParam.getY(), 0.001);
    assertEquals(0.0, stringParam.getZ(), 0.001);
    assertEquals(0, stringParam.getColorValue());
    assertEquals(0, stringParam.getFilterMode());
  }

  @Test
  public void testColorFilterModeForNonColorFilter() {
    AnimaXValueParam colorParam = AnimaXValueParam.fromColor(0xFFFF0000);
    assertEquals(0, colorParam.getFilterMode());
  }

  @Test
  public void testColorValueForNonColorTypes() {
    AnimaXValueParam numberParam = AnimaXValueParam.fromNumber(42.0);
    AnimaXValueParam stringParam = AnimaXValueParam.fromString("test");
    AnimaXValueParam boolParam = AnimaXValueParam.fromBoolean(true);

    assertEquals(0, numberParam.getColorValue());
    assertEquals(0, stringParam.getColorValue());
    assertEquals(0, boolParam.getColorValue());
  }

  @Test
  public void testToStringString() {
    AnimaXValueParam param = AnimaXValueParam.fromString("test", 10);
    String result = param.toString();
    assertTrue(result.contains("stringValue='test'"));
    assertTrue(result.contains("targetFrame=10"));
  }

  @Test
  public void testToStringNumber() {
    AnimaXValueParam param = AnimaXValueParam.fromNumber(42.5, 20);
    String result = param.toString();
    assertTrue(result.contains("numberValue=42.5"));
    assertTrue(result.contains("targetFrame=20"));
  }

  @Test
  public void testToStringBoolean() {
    AnimaXValueParam param = AnimaXValueParam.fromBoolean(true, 30);
    String result = param.toString();
    assertTrue(result.contains("booleanValue=true"));
    assertTrue(result.contains("targetFrame=30"));
  }

  @Test
  public void testToStringCoordinate3D() {
    AnimaXValueParam param = AnimaXValueParam.fromCoordinate(10.5, 20.5, 30.5, 40);
    String result = param.toString();
    assertTrue(result.contains("coordinateValue=(10.5,20.5,30.5)"));
    assertTrue(result.contains("targetFrame=40"));
  }

  @Test
  public void testToStringColor() {
    AnimaXValueParam param = AnimaXValueParam.fromColor(0xFFFF0000, 50);
    String result = param.toString();
    assertTrue(result.contains("colorValue=" + 0xFFFF0000));
    assertTrue(result.contains("targetFrame=50"));
  }

  @Test
  public void testToStringColorFilter() {
    AnimaXValueParam param = AnimaXValueParam.fromColorFilter(0xFF00FF00, 3, 60);
    String result = param.toString();
    assertTrue(result.contains("colorValue=" + 0xFF00FF00));
    assertTrue(result.contains("mode=3.0"));
    assertTrue(result.contains("targetFrame=60"));
  }

  @Test
  public void testToStringNull() {
    AnimaXValueParam param = AnimaXValueParam.createNull();
    String result = param.toString();
    assertTrue(result.contains("null"));
    assertTrue(result.contains("targetFrame=-1"));
  }

  @Test
  public void testTargetFrameZero() {
    AnimaXValueParam param = AnimaXValueParam.fromNumber(1.0, 0);
    assertTrue(param.hasTargetFrame());
    assertEquals(0, param.getTargetFrame());
  }

  @Test
  public void testTargetFrameNegative() {
    AnimaXValueParam param = AnimaXValueParam.fromNumber(1.0, -5);
    assertTrue(param.hasTargetFrame());
    assertEquals(-5, param.getTargetFrame());
  }
}
