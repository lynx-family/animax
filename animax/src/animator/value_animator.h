// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_ANIMATOR_VALUE_ANIMATOR_H_
#define ANIMAX_SRC_ANIMATOR_VALUE_ANIMATOR_H_

#include <memory>

#include "src/player/value_animator_listener.h"

namespace lynx {
namespace animax {
class VSyncMonitor;
class ValueAnimator : public std::enable_shared_from_this<ValueAnimator> {
 public:
  static constexpr double kSecondNs = 1000000000.f;
  static constexpr double kDefaultInterval = kSecondNs / 60.f;
  static constexpr int16_t kForceFrames = 100;

  static std::shared_ptr<ValueAnimator> Create(
      std::shared_ptr<VSyncMonitor> vsync_monitor,
      std::weak_ptr<ValueAnimatorListener> listener);

  ~ValueAnimator() = default;

  /**
   * Set loop count for current animation. the default value is 1.
   * @param loop_count a positive number or zero, zero means loop forever. a
   * negative number will be discarded.
   */
  void SetLoopCount(const int32_t loop_count);
  /**
   * Set auto reverse for the even loop. the default value is false.
   * If true, the even loop(counting from 1) will be played from last frame to
   * first frame; the odd loop will be played as usual.
   * @param auto_reverse whether enable auto reverse.
   */
  void SetAutoReverse(const bool auto_reverse);
  /**
   * Set the speed of animation. the default value is 1.
   * @param speed a relative value. Can be positive, negative and 0.
   */
  void SetSpeed(const double speed);
  /**
   * Set max frame rate.
   * This value is different from fr of lottie json file. fr is used to control
   * the speed of value animator, but max frame rate is used to limit rate of
   * callback OnProgress.
   * @param max_frame_rate max frame rate.
   */
  void SetMaxFrameRate(const double max_frame_rate);
  /**
   * Set origin start frame, end frame and frame rate.
   * These values are corresponding to ip, op, fr of lottie json file.
   * @param origin_start_frame ip of lottie json file, means In Point.
   * @param origin_end_frame   op of lottie json file, means Out Point.
   * @param frame_rate         fr of lottie json file, means frame rate.
   */
  void SetOriginFrameProperty(const double origin_start_frame,
                              const double origin_end_frame,
                              const double frame_rate);
  /**
   * Set play segments of animation.
   * You should call this method after calling SetOriginFrameProperty. the
   * default start frame is the origin start frame, the default end frame is the
   * origin end frame.
   * @param start_frame start frame of animation, a value less than origin start
   * frame will be converted to origin start frame.
   * @param end_frame   end frame of animation, a value greater than origin end
   * frame will be converted to origin end frame; -1.0 will be converted to
   * origin end frame too.
   */
  void SetPlaySegments(double start_frame, double end_frame);
  /**
   * Whether the animation is playing.
   * @return true if the inner state is kPlaying.
   */
  bool IsAnimating();
  /**
   * Whether origin start frame, origin end frame and frame rate set by
   * SetOriginFrameProperty are valid. You can call this method after
   * SetOriginFrameProperty.
   * @return true if origin start frame, origin end frame and frame rate are
   * valid.
   */
  bool CanPlay();
  /**
   * Start animation.
   * Resume animation from break point. Currently the only difference between
   * Start and Resume is Start will trigger OnStart event and Resume will not!
   */
  void Start();
  /**
   * Resume animation.
   * Resume animation from break point.
   */
  void Resume();
  /**
   * Pause animation.
   */
  void Pause();
  /**
   * Stop animation.
   * After calling this, value animator resets its progress to 0. A Play() after
   * Stop() will play animation from start frame.
   */
  void Stop();
  /**
   * Seek to a progress.
   * @param progress           a progress between origin start frame and origin
   * end frame.
   * @param current_loop_count modify inner loop count which is used to compare
   * with total loop count.
   */
  void SeekToProgress(double progress, int32_t current_loop_count = 0);
  /**
   * Seek to a frame.
   * @param frame              a frame which will be added to origin start frame
   * @param current_loop_count modify inner loop count which is used to compare
   * with total loop count.
   */
  void SeekToFrame(double frame, int32_t current_loop_count = 0);
  /**
   * Seek to current frame.
   */
  void SeekToCurrentFrame();

 private:
  enum class State : uint8_t {
    kUnknown = 0,
    kPlaying,
    kPaused,
  };
  ValueAnimator(std::shared_ptr<VSyncMonitor> vsync_monitor,
                std::weak_ptr<ValueAnimatorListener> listener);
  void SetupOnFrame();
  void OnFrame(double current_time_ms);
  double GetProgress() const;
  bool IsLoopForever();
  void MakeCurrentFrameValid();
  void ResetState();
  bool UpdateNextFrameNs(double current_time_ms);

  std::shared_ptr<VSyncMonitor> vsync_monitor_;
  std::weak_ptr<ValueAnimatorListener> listener_;
  // Property Begin
  int32_t loop_count_ = 1;
  bool auto_reverse_ = false;
  double speed_ = 1.0;
  double start_frame_ = 0.0;
  double end_frame_ = 0.0;
  double frame_rate_ = 0.0;
  double origin_start_frame_ = 0.0;
  double origin_end_frame_ = 0.0;
  double fps_interval_ns_ = kDefaultInterval;
  // Property End
  // State Begin
  double last_time_ms_ = -1.0;
  int32_t current_loop_count_ = 0;
  double current_frame_ = 0.0;
  bool has_on_start_emit_ = false;
  double next_frame_ns_ = 0.f;
  State state_ = State::kUnknown;
  // State End
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_ANIMATOR_VALUE_ANIMATOR_H_
