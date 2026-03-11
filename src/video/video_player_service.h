// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_VIDEO_PLAYER_SERVICE_H_
#define ANIMAX_SRC_VIDEO_VIDEO_PLAYER_SERVICE_H_

#include <shared_mutex>

#include "src/resource/asset/video_asset.h"
#include "src/video/custom/video_decoder_custom.h"
#include "src/video/video_player.h"
#include "src/video/video_player_provider.h"
#include "src/video/video_player_provider_default.h"
#include "src/video/video_shader.h"

namespace lynx {
namespace animax {

class VideoPlayerService {
 public:
  /**
   * Gets the singleton instance of VideoPlayerService.
   */
  static VideoPlayerService& GetInstance() {
    static base::NoDestructor<VideoPlayerService> instance;
    return *instance;
  }

  /**
   * Registers a video player provider with the service.
   * @param key The name of this provider for logging and identification.
   * @param provider The provider instance to register.
   * @return true if the provider was successfully registered, false if the
   * provider is null.
   */
  bool RegisterProvider(const std::string& key,
                        std::shared_ptr<VideoPlayerProvider> provider);

  /**
   * Gets a video player provider for the given ability and asset.
   * This method finds a registered provider that satisfies the IsSupport
   * condition.
   * @param ability The ability to check support for.
   * @param asset The video asset containing the video data and metadata.
   * @return A pointer to the VideoPlayerProvider instance.
   */
  std::shared_ptr<VideoPlayerProvider> GetProvider(const AnimaXAbility* ability,
                                                   VideoAsset* asset);

 private:
  VideoPlayerService()
      : default_provider_(std::make_shared<VideoPlayerProviderDefault>()) {}
  ~VideoPlayerService() = default;

  friend class base::NoDestructor<VideoPlayerService>;
  mutable std::shared_mutex mutex_;
  std::unordered_map<std::string, std::shared_ptr<VideoPlayerProvider>>
      providers_;
  std::shared_ptr<VideoPlayerProviderDefault> default_provider_;
};

ANIMAX_EXPORT bool RegisterVideoPlayerProvider(
    const char* key, std::shared_ptr<VideoPlayerProvider> provider);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_VIDEO_PLAYER_SERVICE_H_
