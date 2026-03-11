// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import com.lynx.animax.base.CalledByNative;
import java.lang.Number;

/**
 * A value parameter class that can hold different types of values (String, Number, Boolean,
 * Coordinate) for animation parameters. Supports optional target frame for keyframe animations.
 */
public class AnimaXValueParam {
  public static final int FRAME_INDEX_ALL = -1;

  /**
   * A simple class to hold 3D coordinate data (x, y, z).
   */
  private static class Coordinate {
    public final double x;
    public final double y;
    public final double z;

    public Coordinate(double x, double y, double z) {
      this.x = x;
      this.y = y;
      this.z = z;
    }
  }

  private enum Type { STRING, NUMBER, BOOLEAN, COORDINATE, COLOR, COLOR_FILTER, NULL }

  private final Type mType;
  private final String mStringValue;
  private final Number mNumberValue;
  private final Boolean mBooleanValue;
  private final Coordinate mCoordinateValue;
  private final Integer mColorValue;
  private final Integer mTargetFrame;

  private AnimaXValueParam(Type type, String stringValue, Number numberValue, Boolean booleanValue,
      Coordinate coordinateValue, Integer colorValue, Integer targetFrame) {
    this.mType = type;
    this.mStringValue = stringValue;
    this.mNumberValue = numberValue;
    this.mBooleanValue = booleanValue;
    this.mCoordinateValue = coordinateValue;
    this.mColorValue = colorValue;
    this.mTargetFrame = targetFrame;
  }

  /**
   * Creates a AnimaXValueParam from a String value
   *
   * @param value String value
   * @return AnimaXValueParam instance
   */
  @CalledByNative
  public static AnimaXValueParam fromString(String value) {
    return new AnimaXValueParam(Type.STRING, value, null, null, null, null, null);
  }

  /**
   * Creates a AnimaXValueParam from a String value with a target frame
   *
   * @param value String value
   * @param targetFrame Frame number at which this value should be applied
   * @return AnimaXValueParam instance
   */
  public static AnimaXValueParam fromString(String value, int targetFrame) {
    return new AnimaXValueParam(Type.STRING, value, null, null, null, null, targetFrame);
  }

  /**
   * Creates a AnimaXValueParam from a numeric value (int, float, double)
   *
   * @param value Numeric value
   * @return AnimaXValueParam instance
   */
  @CalledByNative
  public static AnimaXValueParam fromNumber(double value) {
    return new AnimaXValueParam(Type.NUMBER, null, value, null, null, null, null);
  }

  /**
   * Creates a AnimaXValueParam from a numeric value with a target frame
   *
   * @param value Numeric value
   * @param targetFrame Frame number at which this value should be applied
   * @return AnimaXValueParam instance
   */
  public static AnimaXValueParam fromNumber(double value, int targetFrame) {
    return new AnimaXValueParam(Type.NUMBER, null, value, null, null, null, targetFrame);
  }

  /**
   * Creates a AnimaXValueParam from a boolean value
   *
   * @param value Boolean value
   * @return AnimaXValueParam instance
   */
  @CalledByNative
  public static AnimaXValueParam fromBoolean(boolean value) {
    return new AnimaXValueParam(Type.BOOLEAN, null, null, value, null, null, null);
  }

  /**
   * Creates a AnimaXValueParam from a boolean value with a target frame
   *
   * @param value Boolean value
   * @param targetFrame Frame number at which this value should be applied
   * @return AnimaXValueParam instance
   */
  public static AnimaXValueParam fromBoolean(boolean value, int targetFrame) {
    return new AnimaXValueParam(Type.BOOLEAN, null, null, value, null, null, targetFrame);
  }

  /**
   * Creates a AnimaXValueParam from a 2D coordinate (x,y coordinates)
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @return AnimaXValueParam instance
   */
  public static AnimaXValueParam fromCoordinate(double x, double y) {
    return new AnimaXValueParam(
        Type.COORDINATE, null, null, null, new Coordinate(x, y, 0), null, null);
  }

  /**
   * Creates a AnimaXValueParam from a 2D coordinate with a target frame
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @param targetFrame Frame number at which this value should be applied
   * @return AnimaXValueParam instance
   */
  public static AnimaXValueParam fromCoordinate(double x, double y, int targetFrame) {
    return new AnimaXValueParam(
        Type.COORDINATE, null, null, null, new Coordinate(x, y, 0), null, targetFrame);
  }

  /**
   * Creates a AnimaXValueParam from a 3D coordinate (x,y,z coordinates)
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @param z Z coordinate
   * @return AnimaXValueParam instance
   */
  @CalledByNative
  public static AnimaXValueParam fromCoordinate(double x, double y, double z) {
    return new AnimaXValueParam(
        Type.COORDINATE, null, null, null, new Coordinate(x, y, z), null, null);
  }

  /**
   * Creates a AnimaXValueParam from a 3D coordinate with a target frame
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @param z Z coordinate
   * @param targetFrame Frame number at which this value should be applied
   * @return AnimaXValueParam instance
   */
  public static AnimaXValueParam fromCoordinate(double x, double y, double z, int targetFrame) {
    return new AnimaXValueParam(
        Type.COORDINATE, null, null, null, new Coordinate(x, y, z), null, targetFrame);
  }

  /**
   * Creates a AnimaXValueParam from a color value
   *
   * @param color Color integer value
   * @return AnimaXValueParam instance
   */
  @CalledByNative
  public static AnimaXValueParam fromColor(int color) {
    return new AnimaXValueParam(Type.COLOR, null, null, null, null, color, null);
  }

  /**
   * Creates a AnimaXValueParam from a color value with a target frame
   *
   * @param color Color integer value
   * @param targetFrame Frame number at which this value should be applied
   * @return AnimaXValueParam instance
   */
  public static AnimaXValueParam fromColor(int color, int targetFrame) {
    return new AnimaXValueParam(Type.COLOR, null, null, null, null, color, targetFrame);
  }

  /**
   * Creates a AnimaXValueParam for a color filter with color and mode
   *
   * @param color Color integer value
   * @param mode Filter mode value
   * @return AnimaXValueParam instance
   */
  @CalledByNative
  public static AnimaXValueParam fromColorFilter(int color, int mode) {
    return new AnimaXValueParam(Type.COLOR_FILTER, null, (double) mode, null, null, color, null);
  }

  /**
   * Creates a AnimaXValueParam for a color filter with color and mode, and a target frame
   *
   * @param color Color integer value
   * @param mode Filter mode value
   * @param targetFrame Frame number at which this value should be applied
   * @return AnimaXValueParam instance
   */
  public static AnimaXValueParam fromColorFilter(int color, int mode, int targetFrame) {
    return new AnimaXValueParam(
        Type.COLOR_FILTER, null, (double) mode, null, null, color, targetFrame);
  }

  /**
   * Creates a null AnimaXValueParam
   *
   * @return AnimaXValueParam instance with null value
   */
  public static AnimaXValueParam createNull() {
    return new AnimaXValueParam(Type.NULL, null, null, null, null, null, null);
  }

  /**
   * Checks if this parameter has a target frame
   *
   * @return true if target frame is specified
   */
  public boolean hasTargetFrame() {
    return mTargetFrame != null && mTargetFrame != FRAME_INDEX_ALL;
  }

  /**
   * Gets the target frame if specified
   *
   * @return Target frame or -1 if not specified
   */
  @CalledByNative
  public int getTargetFrame() {
    return mTargetFrame != null ? mTargetFrame : FRAME_INDEX_ALL;
  }

  /**
   * Checks if the value is a String
   *
   * @return true if value is a String
   */
  public boolean isString() {
    return mType == Type.STRING;
  }

  /**
   * Checks if the value is a Number
   *
   * @return true if value is a Number
   */
  public boolean isNumber() {
    return mType == Type.NUMBER;
  }

  /**
   * Checks if the value is a Boolean
   *
   * @return true if value is a Boolean
   */
  public boolean isBoolean() {
    return mType == Type.BOOLEAN;
  }

  /**
   * Checks if the value is a Coordinate
   *
   * @return true if value is a Coordinate
   */
  public boolean isCoordinate() {
    return mType == Type.COORDINATE;
  }

  /**
   * Checks if the value is a Color
   *
   * @return true if value is a Color
   */
  public boolean isColor() {
    return mType == Type.COLOR;
  }

  /**
   * Checks if the value is a ColorFilter
   *
   * @return true if value is a ColorFilter
   */
  public boolean isColorFilter() {
    return mType == Type.COLOR_FILTER;
  }

  /**
   * Checks if the value is null
   *
   * @return true if value is null
   */
  public boolean isNull() {
    return mType == Type.NULL;
  }

  /**
   * Gets the value as a String
   *
   * @return String value or null if not a String
   */
  @CalledByNative
  public String getStringValue() {
    return mStringValue;
  }

  /**
   * Gets the value as a Number
   *
   * @return Number value or null if not a Number
   */
  @CalledByNative
  public double getNumberValue() {
    return mNumberValue != null ? mNumberValue.doubleValue() : 0.0;
  }

  /**
   * Gets the value as a Boolean
   *
   * @return Boolean value or null if not a Boolean
   */
  @CalledByNative
  public boolean getBooleanValue() {
    return mBooleanValue != null ? mBooleanValue : false;
  }

  /**
   * Gets the X coordinate if this is a Coordinate value
   *
   * @return X coordinate or 0 if not a Coordinate
   */
  @CalledByNative
  public double getX() {
    return mCoordinateValue != null ? mCoordinateValue.x : 0d;
  }

  /**
   * Gets the Y coordinate if this is a Coordinate value
   *
   * @return Y coordinate or 0 if not a Coordinate
   */
  @CalledByNative
  public double getY() {
    return mCoordinateValue != null ? mCoordinateValue.y : 0d;
  }

  /**
   * Gets the Z coordinate if this is a Coordinate value
   *
   * @return Z coordinate or 0 if not a Coordinate
   */
  @CalledByNative
  public double getZ() {
    return mCoordinateValue != null ? mCoordinateValue.z : 0d;
  }

  /**
   * Gets the color value
   *
   * @return Color int value or 0 if not a Color or ColorFilter
   */
  @CalledByNative
  public int getColorValue() {
    if (isColor() || isColorFilter()) {
      return mColorValue != null ? mColorValue : 0;
    }
    return 0;
  }

  /**
   * Gets the filter mode value if this is a ColorFilter
   *
   * @return Mode value or 0 if not a ColorFilter
   */
  @CalledByNative
  public int getFilterMode() {
    return isColorFilter() ? (int) getNumberValue() : 0;
  }

  @CalledByNative
  public int getTypeIndex() {
    return mType.ordinal();
  }

  @Override
  public String toString() {
    int targetFrame = getTargetFrame();
    switch (mType) {
      case STRING:
        return "AnimaXValueParam{stringValue='" + mStringValue + "', targetFrame=" + targetFrame
            + "}";
      case NUMBER:
        return "AnimaXValueParam{numberValue=" + getNumberValue() + ", targetFrame=" + targetFrame
            + "}";
      case BOOLEAN:
        return "AnimaXValueParam{booleanValue=" + getBooleanValue() + ", targetFrame=" + targetFrame
            + "}";
      case COORDINATE:
        return "AnimaXValueParam{coordinateValue=(" + getX() + "," + getY() + "," + getZ()
            + "), targetFrame=" + targetFrame + "}";
      case COLOR:
        return "AnimaXValueParam{colorValue=" + getColorValue() + ", targetFrame=" + targetFrame
            + "}";
      case COLOR_FILTER:
        return "AnimaXValueParam{colorValue=" + getColorValue() + ", mode=" + getNumberValue()
            + ", targetFrame=" + targetFrame + "}";
      case NULL:
        return "AnimaXValueParam{null, targetFrame=" + targetFrame + "}";
      default:
        return "AnimaXValueParam{unknown}";
    }
  }
}
