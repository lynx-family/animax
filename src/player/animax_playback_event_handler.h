// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANIMAX_PLAYBACK_EVENT_HANDLER_H_
#define ANIMAX_SRC_PLAYER_ANIMAX_PLAYBACK_EVENT_HANDLER_H_

#include <memory>

#include "base/include/lynx_actor.h"
#include "src/base/monitor/animax_metrics_listener.h"
#include "src/player/animax_main_controller.h"
#include "src/player/layer_event_listener.h"
#include "src/player/value_animator_listener.h"

namespace lynx {
namespace animax {

struct AnimaXPlayerContext;
class AnimaXPlayer;
class AnimaXRenderer;
class AnimaXMainController;

class AnimaXPlaybackEventHandler
    : public std::enable_shared_from_this<AnimaXPlaybackEventHandler>,
      public ValueAnimatorListener,
      public LayerEventListener,
      public FPSListener {
 public:
  explicit AnimaXPlaybackEventHandler() = default;
  ~AnimaXPlaybackEventHandler() override = default;

  void Init(std::shared_ptr<AnimaXPlayerContext> context);

  // ValueAnimatorListener overrides
  void OnStart() override;
  void OnResume() override;
  void OnPause() override;
  void OnProgress(double progress, double current_frame,
                  bool skippable) override;
  void OnNewLoop(int32_t current_loop) override;
  void OnEnd() override;
  void OnCancel() override;
  void OnWarning(const EventWarning warning,
                 const std::string& warning_msg) override;

  // LayerEventListener overrides
  void OnLayerError(const EventError err, const std::string& err_msg) override;
  void OnLayerWarning(const EventWarning warning,
                      const std::string& warning_msg) override;

  // FPSListener overrides
  void OnFps(float fps, uint32_t session_max_drop_value) override;

 private:
  std::weak_ptr<shell::LynxActor<AnimaXMainController>> weak_controller_actor_;
  std::weak_ptr<shell::LynxActor<AnimaXRenderer>> weak_renderer_actor;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANIMAX_PLAYBACK_EVENT_HANDLER_H_
