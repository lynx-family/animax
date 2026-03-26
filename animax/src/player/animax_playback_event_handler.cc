// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/animax_playback_event_handler.h"

#include "include/player/animax_event.h"
#include "include/player/animax_player.h"
#include "src/base/log/log.h"
#include "src/player/animax_main_controller.h"
#include "src/player/animax_player_context.h"
#include "src/player/animax_renderer.h"

namespace lynx {
namespace animax {

void AnimaXPlaybackEventHandler::Init(
    std::shared_ptr<AnimaXPlayerContext> context) {
  DCHECK(context);
  weak_controller_actor_ = context->weak_main_controller;
  weak_renderer_actor = context->weak_renderer_actor;
}

void AnimaXPlaybackEventHandler::OnStart() {
  auto controller_actor = weak_controller_actor_.lock();
  if (controller_actor) {
    controller_actor->Act([](auto& controller) {
      controller->NotifyCurrentFrameEvent(Event::kStart);
    });
  }
}

void AnimaXPlaybackEventHandler::OnResume() {
  auto renderer_actor = weak_renderer_actor.lock();
  if (renderer_actor) {
    renderer_actor->Act([](auto& renderer) { renderer->OnResume(); });
  }
}

void AnimaXPlaybackEventHandler::OnPause() {
  auto renderer_actor = weak_renderer_actor.lock();
  if (renderer_actor) {
    renderer_actor->Act([](auto& renderer) { renderer->OnPause(); });
  }
}

void AnimaXPlaybackEventHandler::OnProgress(double progress,
                                            double current_frame) {
  auto controller_actor = weak_controller_actor_.lock();
  if (controller_actor) {
    controller_actor->Act([progress, current_frame](auto& controller) {
      controller->OnProgress(progress, current_frame);
    });
  }
}

void AnimaXPlaybackEventHandler::OnNewLoop(int32_t current_loop) {
  auto controller_actor = weak_controller_actor_.lock();
  if (controller_actor) {
    controller_actor->Act([current_loop](auto& controller) {
      controller->OnNewLoop(current_loop);
    });
  }
}

void AnimaXPlaybackEventHandler::OnEnd() {
  auto renderer_actor = weak_renderer_actor.lock();
  if (renderer_actor) {
    renderer_actor->Act([](auto& renderer) { renderer->OnEnd(); });
  }
  auto controller_actor = weak_controller_actor_.lock();
  if (controller_actor) {
    controller_actor->Act([](auto& controller) { controller->OnEnd(); });
  }
}

void AnimaXPlaybackEventHandler::OnCancel() {
  auto renderer_actor = weak_renderer_actor.lock();
  if (renderer_actor) {
    renderer_actor->Act([](auto& renderer) { renderer->OnCancel(); });
  }
  auto controller_actor = weak_controller_actor_.lock();
  if (controller_actor) {
    controller_actor->Act([](auto& controller) {
      controller->NotifyCurrentFrameEvent(Event::kCancel);
    });
  }
}

void AnimaXPlaybackEventHandler::OnWarning(const EventWarning warning,
                                           const std::string& warning_msg) {
  auto controller_actor = weak_controller_actor_.lock();
  if (controller_actor) {
    controller_actor->Act([warning, warning_msg](auto& controller) {
      controller->NotifyWarning(warning, warning_msg);
    });
  }
}

void AnimaXPlaybackEventHandler::OnLayerError(const EventError err,
                                              const std::string& err_msg) {
  auto controller_actor = weak_controller_actor_.lock();
  if (controller_actor) {
    controller_actor->Act([err, err_msg](auto& controller) {
      controller->NotifyError(err, err_msg);
    });
  }
}

void AnimaXPlaybackEventHandler::OnLayerWarning(
    const EventWarning warning, const std::string& warning_msg) {
  OnWarning(warning, warning_msg);
}

void AnimaXPlaybackEventHandler::OnFps(float fps,
                                       uint32_t session_max_drop_value) {
  auto controller_actor = weak_controller_actor_.lock();
  if (controller_actor) {
    controller_actor->Act([fps, session_max_drop_value](auto& controller) {
      controller->NotifyFpsEvent(fps, session_max_drop_value);
    });
  }
}

}  // namespace animax
}  // namespace lynx
