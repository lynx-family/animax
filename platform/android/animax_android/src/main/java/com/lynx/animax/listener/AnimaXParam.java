// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.listener;

import androidx.annotation.NonNull;
import java.util.Map;

public class AnimaXParam {
  private static final String KEY_ANIMATION_ID = "animationID";
  private static final String KEY_CURRENT = "current";
  private static final String KEY_TOTAL = "total";
  private static final String KEY_LOOP_INDEX = "loopIndex";
  private static final String KEY_ELEMENT_ID = "elementID";

  private Map<String, Object> mOriginParams;

  /**
   * Constructor to create an event with animation data.
   * @param originParams A map containing the original parameters with event information.
   */
  public AnimaXParam(@NonNull Map<String, Object> originParams) {
    this.mOriginParams = originParams;
  }

  /**
   * Retrieves the unique identifier for the animation.
   * @return The animation ID as a string.
   */
  public String getAnimationID() {
    return getString(KEY_ANIMATION_ID);
  }

  /**
   * Retrieves the current frame number of the animation.
   * @return The current frame as a float.
   */
  public float getCurrentFrame() {
    return getFloat(KEY_CURRENT);
  }

  /**
   * Retrieves the total number of frames in the animation.
   * @return The total frames as a float.
   */
  public float getTotalFrame() {
    return getFloat(KEY_TOTAL);
  }

  /**
   * Retrieves the current loop index for animations that repeat.
   * @return The loop index as an integer.
   */
  public int getLoopIndex() {
    return getInt(KEY_LOOP_INDEX);
  }

  /**
   * Provides access to the original event parameters.
   * @return A map of parameter keys to their respective values.
   */
  public Map<String, Object> getOriginParams() {
    return mOriginParams;
  }

  /**
   * Utility method to retrieve an integer value from the event parameters.
   * @param key The key for the desired parameter.
   * @return The integer value or -1 if not found or not an integer.
   */
  protected int getInt(String key) {
    Object value = mOriginParams.get(key);
    return value instanceof Number ? ((Number) value).intValue() : -1;
  }

  /**
   * Utility method to retrieve a float value from the event parameters.
   * @param key The key for the desired parameter.
   * @return The float value or -1.0 if not found or not a float.
   */
  protected float getFloat(String key) {
    Object value = mOriginParams.get(key);
    return value instanceof Number ? ((Number) value).floatValue() : -1;
  }

  /**
   * Utility method to retrieve a string value from the event parameters.
   * @param key The key for the desired parameter.
   * @return The string value or null if not found or not a string.
   */
  protected String getString(String key) {
    Object value = mOriginParams.get(key);
    return value instanceof String ? (String) value : null;
  }
}
