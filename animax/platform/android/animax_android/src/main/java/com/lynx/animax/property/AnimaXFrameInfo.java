// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import androidx.annotation.FloatRange;
import androidx.annotation.Nullable;

/**
 * Contains information about the current animation frame for value callbacks.
 */
public class AnimaXFrameInfo {
  private final float mStartFrame;
  private final float mEndFrame;
  @Nullable private final AnimaXValueParam mStartValue;
  @Nullable private final AnimaXValueParam mEndValue;
  @FloatRange(from = 0f, to = 1f) private final float mLinearProgress;
  @FloatRange(from = 0f, to = 1f) private final float mInterpolatedProgress;
  @FloatRange(from = 0f, to = 1f) private final float mOverallProgress;

  /**
   * Create a new frame info object
   *
   * @param startFrame The composition start frame
   * @param endFrame The composition end frame
   * @param startValue The starting value of the current keyframe
   * @param endValue The ending value of the current keyframe
   * @param linearProgress The linear (non-interpolated) progress in the current keyframe (0-1)
   * @param interpolatedProgress The interpolated progress in the current keyframe (0-1)
   * @param overallProgress The overall progress of the animation (0-1)
   */
  public AnimaXFrameInfo(float startFrame, float endFrame, @Nullable AnimaXValueParam startValue,
      @Nullable AnimaXValueParam endValue, @FloatRange(from = 0f, to = 1f) float linearProgress,
      @FloatRange(from = 0f, to = 1f) float interpolatedProgress,
      @FloatRange(from = 0f, to = 1f) float overallProgress) {
    mStartFrame = startFrame;
    mEndFrame = endFrame;
    mStartValue = startValue;
    mEndValue = endValue;
    mLinearProgress = linearProgress;
    mInterpolatedProgress = interpolatedProgress;
    mOverallProgress = overallProgress;
  }

  /**
   * Get the starting frame of the composition
   */
  public float getStartFrame() {
    return mStartFrame;
  }

  /**
   * Get the ending frame of the composition
   */
  public float getEndFrame() {
    return mEndFrame;
  }

  /**
   * Get the starting value of the current keyframe
   */
  @Nullable
  public AnimaXValueParam getStartValue() {
    return mStartValue;
  }

  /**
   * Get the ending value of the current keyframe
   */
  @Nullable
  public AnimaXValueParam getEndValue() {
    return mEndValue;
  }

  /**
   * Get the linear (non-interpolated) progress in the current keyframe (0-1)
   */
  public float getLinearProgress() {
    return mLinearProgress;
  }

  /**
   * Get the interpolated progress in the current keyframe (0-1)
   */
  public float getInterpolatedProgress() {
    return mInterpolatedProgress;
  }

  /**
   * Get the overall progress of the animation (0-1)
   */
  public float getOverallProgress() {
    return mOverallProgress;
  }

  /**
   * Returns a string representation of the frame info.
   * Useful for debugging and logging.
   */
  @Override
  public String toString() {
    return "AnimaXFrameInfo{"
        + "frames=" + mStartFrame + "->" + mEndFrame
        + ", startValue=" + (mStartValue != null ? mStartValue : "null")
        + ", endValue=" + (mEndValue != null ? mEndValue : "null")
        + ", progress=[linear=" + String.format("%.2f", mLinearProgress)
        + ", interpolated=" + String.format("%.2f", mInterpolatedProgress)
        + ", overall=" + String.format("%.2f", mOverallProgress) + "]}";
  }
}
