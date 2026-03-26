// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_VALUE_ANIMATOR_LISTENER_H_
#define ANIMAX_SRC_PLAYER_VALUE_ANIMATOR_LISTENER_H_

#include <cstdint>

#include "include/player/animax_event.h"

namespace lynx {
namespace animax {

class ValueAnimatorListener {
 public:
  virtual ~ValueAnimatorListener() = default;

  /**
   * Notify the start event.
   * You will be notified every time you call ValueAnimator::Play()
   */
  virtual void OnStart() = 0;
  /**
   * Notify the animation resumes.
   * You will be notified every time you call ValueAnimator::Play() and
   * ValueAnimator::Resume()
   */
  virtual void OnResume() = 0;
  /**
   * Notify the animation pauses.
   * You will be notified every time you call ValueAnimator::Pause()
   */
  virtual void OnPause() = 0;
  /**
   * Notify new frame comes.
   * You will be notified every frame.
   * @param progress      a progress between origin start frame and origin end
   * frame. 0.0 means origin start frame, 1.0 means origin end frame.
   * @param current_frame current frame. This value is related with progress.
   */
  virtual void OnProgress(double progress, double current_frame) = 0;
  /**
   * Notify entering to new loop.
   * The first turn wouldn't trigger this event, that is current_loop = 0. For
   * instance, you set loop count to 3, then you get OnStart(), OnNewLoop(1),
   * OnNewLoop(2) for each turn.
   * @param current_loop current loop counting from 0, the first turn wouldn't
   * trigger this.
   */
  virtual void OnNewLoop(int32_t current_loop) = 0;
  /**
   * Notify the end event.
   * You will be notified when the value animator finish, there are not any
   * other events more.
   */
  virtual void OnEnd() = 0;
  /**
   * Notify animation cancel event.
   * You will be notified when you stop a playing animation. Including changing
   * source from a playing animation.
   */
  virtual void OnCancel() = 0;
  /**
   * Notify animation warning event.
   * You will be notified when you meet some misuse problems.
   */
  virtual void OnWarning(const EventWarning warning,
                         const std::string &warning_msg) = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_VALUE_ANIMATOR_LISTENER_H_
