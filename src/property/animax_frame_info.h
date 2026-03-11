// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_ANIMAX_FRAME_INFO_H_
#define ANIMAX_SRC_PROPERTY_ANIMAX_FRAME_INFO_H_

namespace lynx {
namespace animax {

class Value;

/**
 * Contains information about the current animation frame for value callbacks.
 * Similar to Lottie's LottieFrameInfo, this provides context about where in
 * the animation timeline a callback is being invoked.
 */
class AnimaXFrameInfo {
 public:
  /**
   * Creates a new frame info object with the specified properties
   *
   * @param start_frame The start frame of the composition
   * @param end_frame The end frame of the composition
   * @param current_frame The current frame being rendered
   * @param start_value The start value of the keyframe
   * @param end_value The end value of the keyframe
   * @param linear_progress The linear (non-interpolated) progress in the
   * current keyframe (0-1)
   * @param interpolated_progress The interpolated progress in the current
   * keyframe (0-1)
   * @param overall_progress The overall progress of the animation (0-1)
   */
  AnimaXFrameInfo(float start_frame, float end_frame, float current_frame,
                  const Value* start_value, const Value* end_value,
                  float linear_progress, float interpolated_progress,
                  float overall_progress)
      : start_frame_(start_frame),
        end_frame_(end_frame),
        current_frame_(current_frame),
        start_value_(start_value),
        end_value_(end_value),
        linear_progress_(linear_progress),
        interpolated_progress_(interpolated_progress),
        overall_progress_(overall_progress) {}

  /**
   * Creates a new frame info object without start and end values
   */
  AnimaXFrameInfo(float start_frame, float end_frame, float current_frame,
                  float linear_progress, float interpolated_progress,
                  float overall_progress)
      : start_frame_(start_frame),
        end_frame_(end_frame),
        current_frame_(current_frame),
        start_value_(nullptr),
        end_value_(nullptr),
        linear_progress_(linear_progress),
        interpolated_progress_(interpolated_progress),
        overall_progress_(overall_progress) {}

  /**
   * Get the starting frame of the composition
   */
  float GetStartFrame() const { return start_frame_; }

  /**
   * Get the ending frame of the composition
   */
  float GetEndFrame() const { return end_frame_; }

  /**
   * Get the current frame being rendered
   */
  float GetCurrentFrame() const { return current_frame_; }

  /**
   * Get the start value of the keyframe
   */
  const Value* GetStartValue() const { return start_value_; }

  /**
   * Get the end value of the keyframe
   */
  const Value* GetEndValue() const { return end_value_; }

  /**
   * Get the linear (non-interpolated) progress in the current keyframe (0-1)
   */
  float GetLinearProgress() const { return linear_progress_; }

  /**
   * Get the interpolated progress in the current keyframe (0-1)
   */
  float GetInterpolatedProgress() const { return interpolated_progress_; }

  /**
   * Get the overall progress of the animation (0-1)
   */
  float GetOverallProgress() const { return overall_progress_; }

 private:
  const float start_frame_;
  const float end_frame_;
  const float current_frame_;
  const Value* start_value_;
  const Value* end_value_;
  const float linear_progress_;
  const float interpolated_progress_;
  const float overall_progress_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_ANIMAX_FRAME_INFO_H_
