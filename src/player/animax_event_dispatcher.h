// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANIMAX_EVENT_DISPATCHER_H_
#define ANIMAX_SRC_PLAYER_ANIMAX_EVENT_DISPATCHER_H_

#include <memory>
#include <unordered_set>
#include <vector>

#include "base/include/closure.h"
#include "include/player/animax_event.h"

namespace lynx {
namespace animax {

class AnimaXPlayer;
class IEventParams;
class AnimaXMainController;

class AnimaXEventDispatcher {
 public:
  explicit AnimaXEventDispatcher(std::weak_ptr<AnimaXPlayer> weak_player,
                                 AnimaXMainController& controller);
  ~AnimaXEventDispatcher() = default;

  void AddEventListener(EventListener listener);
  void ClearEventListeners();

  // Subscription management
  void SubscribeUpdateEvent(int32_t frame);
  void UnsubscribeUpdateEvent(int32_t frame);
  void SubscribeUpdateEvents(const std::unordered_set<int32_t>& frames,
                             bool subscribe);
  void ClearPrevNotifiedFrame();

  /**
   * Notify an error event with message.
   * @param err Error code.
   * @param err_msg Error message string.
   */
  void NotifyError(const EventError err, const std::string& err_msg);
  /**
   * Notify a tap event on specified layers.
   * @param hit_layers Set of layer names that were tapped.
   */
  void NotifyTap(const std::unordered_set<std::string>& hit_layers);
  /**
   * Notify a warning event with message.
   * @param warning Warning code.
   * @param warning_msg Warning message string.
   */
  void NotifyWarning(const EventWarning warning,
                     const std::string& warning_msg);
  /**
   * Notify a frame event with current frame.
   * @param event Event type.
   * @param current_frame Current frame number.
   */
  void NotifyFrameEvent(Event event, double current_frame);
  /**
   * Notify an fps event with current frame, max drop value and fps.
   * @param current_frame Current frame number.
   * @param session_max_drop_value Max drop value in current session.
   * @param fps Current fps.
   */
  void NotifyFps(double current_frame, uint32_t session_max_drop_value,
                 float fps);
  /**
   * Notify an update event with current frame.
   * @param current_frame Current frame number.
   */
  void NotifyUpdateEvent(double current_frame);

  void SetSpeed(double speed);

 private:
  /**
   * Notify an internal event with optional parameters.
   * @param event Event type.
   * @param params Unique pointer to event parameters.
   */
  void NotifyEvent(Event event, std::unique_ptr<IEventParams> params);
  void MarkEvent(Event event);
  EventParamMap ConvertEventParamsIntoMap(
      const Event event, std::unique_ptr<IEventParams> params_in);

  std::weak_ptr<AnimaXPlayer> weak_player_;
  std::vector<EventListener> event_listeners_;
  std::unordered_set<int32_t> subscribed_frames_;
  int32_t prev_notified_frame_ = -1;
  double speed_ = 1.0;

  AnimaXMainController& controller_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANIMAX_EVENT_DISPATCHER_H_
