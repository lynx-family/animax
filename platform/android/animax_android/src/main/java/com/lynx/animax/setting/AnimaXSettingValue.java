// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.setting;

import androidx.annotation.Nullable;
import java.util.Collection;
import java.util.Collections;

/**
 * A type-safe wrapper for AnimaX setting values.
 * Supports String, Collection<String>, Boolean, Long, and Double types.
 */
public final class AnimaXSettingValue {
  private final String mStringValue;
  private final Collection<String> mCollectionValue;
  private final Boolean mBooleanValue;
  private final Long mLongValue;
  private final Double mDoubleValue;
  private final SettingType mType;

  private enum SettingType { STRING, COLLECTION, BOOLEAN, LONG, DOUBLE }

  private AnimaXSettingValue(String value) {
    this.mStringValue = value;
    this.mCollectionValue = null;
    this.mBooleanValue = null;
    this.mLongValue = null;
    this.mDoubleValue = null;
    this.mType = SettingType.STRING;
  }

  private AnimaXSettingValue(Collection<String> value) {
    this.mStringValue = null;
    this.mCollectionValue = value;
    this.mBooleanValue = null;
    this.mLongValue = null;
    this.mDoubleValue = null;
    this.mType = SettingType.COLLECTION;
  }

  private AnimaXSettingValue(Boolean value) {
    this.mStringValue = null;
    this.mCollectionValue = null;
    this.mBooleanValue = value;
    this.mLongValue = null;
    this.mDoubleValue = null;
    this.mType = SettingType.BOOLEAN;
  }

  private AnimaXSettingValue(Long value) {
    this.mStringValue = null;
    this.mCollectionValue = null;
    this.mBooleanValue = null;
    this.mLongValue = value;
    this.mDoubleValue = null;
    this.mType = SettingType.LONG;
  }

  private AnimaXSettingValue(Double value) {
    this.mStringValue = null;
    this.mCollectionValue = null;
    this.mBooleanValue = null;
    this.mLongValue = null;
    this.mDoubleValue = value;
    this.mType = SettingType.DOUBLE;
  }

  /**
   * Creates a setting value from a string.
   *
   * @param value The string value, null will be converted to empty string
   * @return A new AnimaXSettingValue instance containing the string value
   */
  public static AnimaXSettingValue fromString(@Nullable String value) {
    return new AnimaXSettingValue(value != null ? value : "");
  }

  /**
   * Creates a setting value from a collection of strings.
   *
   * @param value The collection of strings, null will be converted to empty collection
   * @return A new AnimaXSettingValue instance containing the collection
   */
  public static AnimaXSettingValue fromCollection(@Nullable Collection<String> value) {
    return new AnimaXSettingValue(value != null ? value : Collections.emptyList());
  }

  /**
   * Creates a setting value from a boolean.
   *
   * @param value The boolean value
   * @return A new AnimaXSettingValue instance containing the boolean value
   */
  public static AnimaXSettingValue fromBoolean(boolean value) {
    return new AnimaXSettingValue(value);
  }

  /**
   * Creates a setting value from a long.
   *
   * @param value The long value
   * @return A new AnimaXSettingValue instance containing the long value
   */
  public static AnimaXSettingValue fromLong(long value) {
    return new AnimaXSettingValue(value);
  }

  /**
   * Creates a setting value from a double.
   *
   * @param value The double value
   * @return A new AnimaXSettingValue instance containing the double value
   */
  public static AnimaXSettingValue fromDouble(double value) {
    return new AnimaXSettingValue(value);
  }

  /**
   * Creates an empty setting value.
   *
   * @return A new empty AnimaXSettingValue instance with empty string value
   */
  public static AnimaXSettingValue empty() {
    return new AnimaXSettingValue("");
  }

  public boolean isString() {
    return mType == SettingType.STRING;
  }

  public boolean isCollection() {
    return mType == SettingType.COLLECTION;
  }

  public boolean isBoolean() {
    return mType == SettingType.BOOLEAN;
  }

  public boolean isLong() {
    return mType == SettingType.LONG;
  }

  public boolean isDouble() {
    return mType == SettingType.DOUBLE;
  }

  public String getStringOrEmpty() {
    return mType == SettingType.STRING ? mStringValue : "";
  }

  public Collection<String> getCollectionOrEmpty() {
    return mType == SettingType.COLLECTION ? mCollectionValue : Collections.emptyList();
  }

  public boolean getBooleanOrFalse() {
    return mType == SettingType.BOOLEAN && mBooleanValue != null ? mBooleanValue : false;
  }

  public long getLongOrZero() {
    return mType == SettingType.LONG && mLongValue != null ? mLongValue : 0L;
  }

  public double getDoubleOrZero() {
    return mType == SettingType.DOUBLE && mDoubleValue != null ? mDoubleValue : 0.0;
  }

  /**
   * Returns a string representation of this setting value.
   * Format: {type=<type>, value=<value>}
   * Examples:
   * - String: {type=STRING, value=hello}
   * - Collection: {type=COLLECTION, value=[item1, item2]}
   * - Boolean: {type=BOOLEAN, value=true}
   * - Long: {type=LONG, value=123}
   * - Double: {type=DOUBLE, value=123.45}
   *
   * @return A string representation of the setting value including its type and value
   */
  @Override
  public String toString() {
    StringBuilder builder = new StringBuilder();
    builder.append("{type=").append(mType.name());
    builder.append(", value=");

    switch (mType) {
      case STRING:
        builder.append(mStringValue != null ? mStringValue : "");
        break;
      case COLLECTION:
        if (mCollectionValue != null) {
          builder.append('[').append(String.join(", ", mCollectionValue)).append(']');
        } else {
          builder.append("[]");
        }
        break;
      case BOOLEAN:
        builder.append(mBooleanValue != null ? mBooleanValue : "false");
        break;
      case LONG:
        builder.append(mLongValue != null ? mLongValue : "0");
        break;
      case DOUBLE:
        builder.append(mDoubleValue != null ? mDoubleValue : "0.0");
        break;
      default:
        builder.append("");
    }

    builder.append("}");
    return builder.toString();
  }
}
