// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PLAYER_ANIMAX_PLAYER_BUILDER_H_
#define ANIMAX_INCLUDE_PLAYER_ANIMAX_PLAYER_BUILDER_H_

#include <memory>
#include <vector>

#include "include/base/macros.h"
#include "include/player/animax_event.h"
#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "include/resource/unzip_task.h"

namespace lynx {
namespace animax {

class VSyncMonitor;
class AnimaXAbility;
class AnimaXPlayer;

class ANIMAX_EXPORT AnimaXPlayerBuilder {
 public:
  AnimaXPlayerBuilder() = default;
  AnimaXPlayerBuilder(const AnimaXPlayerBuilder&) = delete;
  AnimaXPlayerBuilder& operator=(const AnimaXPlayerBuilder&) = delete;
  AnimaXPlayerBuilder(AnimaXPlayerBuilder&&) = default;
  AnimaXPlayerBuilder& operator=(AnimaXPlayerBuilder&&) = default;
  /**
   * Set screen scale factor used by the player.
   */
  AnimaXPlayerBuilder& SetScale(float scale);
  /**
   * Set the VSync monitor to drive rendering; required before Build.
   */
  AnimaXPlayerBuilder& SetVSyncMonitor(
      std::shared_ptr<VSyncMonitor> vsync_monitor);
  AnimaXPlayerBuilder& EnableMultiThreadAccelerate(bool enable);
  AnimaXPlayerBuilder& SetSkipCountDownEvent(bool skip);
  /**
   * Set the ability object for the player.
   * Previously configured via AnimaXPlayer::SetAbility.
   */
  AnimaXPlayerBuilder& SetAbility(std::shared_ptr<AnimaXAbility> ability);

  /**
   * Register resource loader and unzip loader for AnimaXPlayer.
   * Any method that uses composition_model_pipeline_ should be called after
   * this. For example: SetJson, SetSrc, SetSrcPolyfill, Play.
   * Previously configured via AnimaXPlayer::RegisterLoaders.
   */
  AnimaXPlayerBuilder& SetResourceLoader(
      Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader);
  AnimaXPlayerBuilder& SetUnzipLoader(
      Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader);

  /**
   * Add a listener to receive internal events from AnimaXPlayer.
   * This should be called before using AnimaXPlayer if you want to listen to
   * events. Previously configured via AnimaXPlayer::AddEventListener.
   */
  AnimaXPlayerBuilder& AddEventListener(EventListener listener);

  /**
   * Create an instance of AnimaXPlayer using configured parameters.
   * @return Shared pointer to the created AnimaXPlayer instance.
   */
  std::shared_ptr<AnimaXPlayer> Build();

 private:
  friend class AnimaXPlayer;

  float scale_ = 1.f;
  std::shared_ptr<VSyncMonitor> vsync_monitor_{};
  bool multi_thread_accelerate_ = false;
  bool skip_count_down_event_ = false;
  std::shared_ptr<AnimaXAbility> ability_{};
  Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader_{};
  Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader_{};
  std::vector<EventListener> event_listeners_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PLAYER_ANIMAX_PLAYER_BUILDER_H_
