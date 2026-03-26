// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/player/animax_player_builder.h"

#include <utility>

#include "include/player/animax_player.h"

namespace lynx {
namespace animax {

AnimaXPlayerBuilder& AnimaXPlayerBuilder::SetScale(float scale) {
  scale_ = scale;
  return *this;
}

AnimaXPlayerBuilder& AnimaXPlayerBuilder::SetVSyncMonitor(
    std::shared_ptr<VSyncMonitor> vsync_monitor) {
  vsync_monitor_ = std::move(vsync_monitor);
  return *this;
}

AnimaXPlayerBuilder& AnimaXPlayerBuilder::EnableMultiThreadAccelerate(
    bool enable) {
  multi_thread_accelerate_ = enable;
  return *this;
}

AnimaXPlayerBuilder& AnimaXPlayerBuilder::DisablePlaybackOnAssetLoadFailure(
    bool disable) {
  disable_playback_on_asset_load_failure_ = disable;
  return *this;
}

AnimaXPlayerBuilder& AnimaXPlayerBuilder::SetAbility(
    std::shared_ptr<AnimaXAbility> ability) {
  ability_ = std::move(ability);
  return *this;
}

AnimaXPlayerBuilder& AnimaXPlayerBuilder::SetResourceLoader(
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader) {
  resource_loader_ = std::move(resource_loader);
  return *this;
}

AnimaXPlayerBuilder& AnimaXPlayerBuilder::SetUnzipLoader(
    Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader) {
  unzip_loader_ = std::move(unzip_loader);
  return *this;
}

AnimaXPlayerBuilder& AnimaXPlayerBuilder::AddEventListener(
    EventListener listener) {
  event_listeners_.push_back(std::move(listener));
  return *this;
}

std::shared_ptr<AnimaXPlayer> AnimaXPlayerBuilder::Build() {
  auto player = std::shared_ptr<AnimaXPlayer>(new AnimaXPlayer(*this));
  player->Init(*this);
  return player;
}

}  // namespace animax
}  // namespace lynx
