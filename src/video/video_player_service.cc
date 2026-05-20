// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/video_player_service.h"

#include "include/player/animax_player.h"
#include "src/base/log/log.h"
#include "src/video/custom/video_player_custom.h"
#include "src/video/video_player_provider_default.h"

namespace lynx {
namespace animax {

ANIMAX_EXPORT bool RegisterVideoPlayerProvider(
    const char* key, std::shared_ptr<VideoPlayerProvider> provider) {
  return VideoPlayerService::GetInstance().RegisterProvider(std::string(key),
                                                            provider);
}

bool VideoPlayerService::RegisterProvider(
    const std::string& key, std::shared_ptr<VideoPlayerProvider> provider) {
  if (key.empty() || !provider) {
    return false;
  }

  std::unique_lock lock(mutex_);

  if (providers_.find(key) != providers_.end()) {
    ANIMAX_LOGW("VideoPlayerService: Provider with key already exists: " + key);
    return false;
  }

  auto [it, inserted] = providers_.emplace(key, std::move(provider));
  if (inserted) {
    ANIMAX_LOGI("VideoPlayerService: Registered provider: " + key);
  }
  return inserted;
}

std::shared_ptr<VideoPlayerProvider> VideoPlayerService::GetProvider(
    std::shared_ptr<AnimaXAbility> ability, std::shared_ptr<VideoAsset> asset) {
  if (ability && asset) {
    std::shared_lock lock(mutex_);
    for (const auto& [key, provider] : providers_) {
      if (provider->IsSupported(ability.get(), asset.get())) {
        ANIMAX_LOGI("VideoPlayerService: get provider: " + key);
        return provider;
      }
    }
  }

  return default_provider_;
}

}  // namespace animax
}  // namespace lynx
