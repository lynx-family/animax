// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/animax_event_dispatcher.h"

#include <cmath>

#include "include/player/animax_player.h"
#include "src/base/log/log.h"
#include "src/base/util/event_warning_checker.h"
#include "src/player/animax_main_controller.h"

namespace lynx {
namespace animax {

AnimaXEventDispatcher::AnimaXEventDispatcher(
    std::weak_ptr<AnimaXPlayer> weak_player, AnimaXMainController& controller)
    : weak_player_(std::move(weak_player)), controller_(controller) {}

void AnimaXEventDispatcher::AddEventListener(EventListener listener) {
  event_listeners_.push_back(std::move(listener));
}

void AnimaXEventDispatcher::ClearEventListeners() { event_listeners_.clear(); }

void AnimaXEventDispatcher::SubscribeUpdateEvent(int32_t frame) {
  subscribed_frames_.insert(frame);
}

void AnimaXEventDispatcher::UnsubscribeUpdateEvent(int32_t frame) {
  subscribed_frames_.erase(frame);
}

void AnimaXEventDispatcher::SubscribeUpdateEvents(
    const std::unordered_set<int32_t>& frames, bool subscribe) {
  if (subscribe) {
    subscribed_frames_.insert(frames.begin(), frames.end());
  } else {
    for (const auto& frame : frames) {
      subscribed_frames_.erase(frame);
    }
  }
}

void AnimaXEventDispatcher::ClearPrevNotifiedFrame() {
  prev_notified_frame_ = -1;
}

void AnimaXEventDispatcher::NotifyUpdateEvent(double frame) {
  const auto current_frame = static_cast<int32_t>(std::round(frame));
  if (-1 == prev_notified_frame_) {
    prev_notified_frame_ = current_frame;
    if (subscribed_frames_.count(current_frame)) {
      NotifyFrameEvent(Event::kUpdate, current_frame);
    }
  } else if (speed_ < 0.0) {
    for (auto frame = prev_notified_frame_ - 1; frame >= current_frame;
         --frame) {
      prev_notified_frame_ = frame;
      if (subscribed_frames_.count(frame)) {
        NotifyFrameEvent(Event::kUpdate, frame);
      }
    }
  } else {
    // speed >= 0.0
    for (auto frame = prev_notified_frame_ + 1; frame <= current_frame;
         ++frame) {
      prev_notified_frame_ = frame;
      if (subscribed_frames_.count(frame)) {
        NotifyFrameEvent(Event::kUpdate, frame);
      }
    }
  }
}

void AnimaXEventDispatcher::NotifyEvent(Event event,
                                        std::unique_ptr<IEventParams> params) {
  DCHECK(params);
  auto player = weak_player_.lock();
  if (!player) {
    return;
  }

  controller_.MarkEvent(event);
  auto params_map = ConvertEventParamsIntoMap(event, std::move(params));
  for (const auto& listener : event_listeners_) {
    listener(player.get(), event, params_map);
  }
}

EventParamMap AnimaXEventDispatcher::ConvertEventParamsIntoMap(
    const Event event, std::unique_ptr<IEventParams> params_in) {
  EventParamMap params_out;
  if (event == Event::kError) {
    auto* error_params = static_cast<ErrorParams*>(params_in.get());
    params_out[EventKeys::kCode] = error_params->code_;
    params_out[EventKeys::kMessage] = error_params->message_;
  } else if (event == Event::kWarning) {
    auto* warning_params = static_cast<WarningParams*>(params_in.get());
    params_out[EventKeys::kCode] = warning_params->code_;
    params_out[EventKeys::kMessage] = warning_params->message_;
  } else {
    params_out[EventKeys::kAnimationId] = controller_.GetAnimationID();
    auto* frame_params = static_cast<FrameParams*>(params_in.get());
    params_out[EventKeys::kCurrent] = frame_params->current_frame_;
    params_out[EventKeys::kTotal] = controller_.GetTotalFrame();
    params_out[EventKeys::kLoopIndex] = controller_.GetLoopIndex();
    switch (event) {
      case Event::kFps: {
        auto* fps_params = static_cast<FpsParams*>(params_in.get());
        params_out[EventKeys::kMaxDropRate] = fps_params->max_drop_rate_;
        params_out[EventKeys::kFps] = fps_params->fps_;
        break;
      }
      case Event::kReady: {
        params_out[EventKeys::kPlayerId] = controller_.GetPlayerID();
        break;
      }
      case Event::kTapLayer: {
        auto* tap_params = static_cast<TapParams*>(params_in.get());
        auto layers_vec = std::make_unique<std::vector<std::string>>();
        layers_vec->reserve(tap_params->hit_layers_.size());
        for (const auto& layer : tap_params->hit_layers_) {
          layers_vec->push_back(layer);
        }
        params_out[EventKeys::kLayerList] = std::move(layers_vec);
        break;
      }
      default:
        break;
    }
  }
  return params_out;
}

void AnimaXEventDispatcher::NotifyError(const EventError err,
                                        const std::string& err_msg) {
  auto params = std::unique_ptr<ErrorParams>(new ErrorParams(err, err_msg));
  NotifyEvent(Event::kError, std::move(params));
}

void AnimaXEventDispatcher::NotifyTap(
    const std::unordered_set<std::string>& hit_layers) {
  auto params = std::unique_ptr<TapParams>(new TapParams(hit_layers));
  NotifyEvent(Event::kTapLayer, std::move(params));
}

void AnimaXEventDispatcher::NotifyWarning(const EventWarning warning,
                                          const std::string& warning_msg) {
  auto player = weak_player_.lock();
  if (!player) {
    return;
  }

  std::ostringstream oss;
  oss << "[animax-view] " << warning_msg
      << " source: " << controller_.GetCurrentSrc();
  auto params =
      std::unique_ptr<WarningParams>(new WarningParams(warning, oss.str()));
  NotifyEvent(Event::kWarning, std::move(params));
}

void AnimaXEventDispatcher::NotifyFrameEvent(Event event,
                                             double current_frame) {
  auto params = std::unique_ptr<FrameParams>(new FrameParams(current_frame));
  NotifyEvent(event, std::move(params));
}

void AnimaXEventDispatcher::NotifyFps(double current_frame,
                                      uint32_t session_max_drop_value,
                                      float fps) {
  auto params = std::unique_ptr<FpsParams>(
      new FpsParams(current_frame, session_max_drop_value, fps));
  NotifyEvent(Event::kFps, std::move(params));
}

void AnimaXEventDispatcher::SetSpeed(double speed) { speed_ = speed; }

}  // namespace animax
}  // namespace lynx
