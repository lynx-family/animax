// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <memory>
#include <string>

#include "base/include/no_destructor.h"
#include "src/base/log/log.h"
#include "src/player/animax_ability.h"
#include "src/player/vsync_dispatcher.h"
#include "src/resource/asset/video_asset.h"
#include "src/resource/asset/video_asset_model.h"
#include "src/resource/resource_loader.h"
#include "src/video/video_player.h"
#include "src/video/video_shader.h"

namespace lynx {
namespace animax {

std::string GetCacheDirectory() { return std::string{}; }

void Log(LogMessage* msg) {}

class UnitTestVideoAsset : public VideoAsset {
 public:
  UnitTestVideoAsset(VideoAssetModel model) : VideoAsset(std::move(model)) {}

  ~UnitTestVideoAsset() override = default;

  bool PrepareFrameData(const std::string& video_path) override {
    is_valid_ = true;
    return is_valid_;
  }
  int32_t GetFrameCount() const override { return 0; }
};

std::shared_ptr<VideoAsset> VideoAsset::Make(VideoAssetModel model) {
  return std::shared_ptr<VideoAsset>(new UnitTestVideoAsset(std::move(model)));
}

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    const AnimaXAbility* ability_ptr) {
  return nullptr;
}

std::unique_ptr<VideoShader> VideoShader::Make(
    const AnimaXAbility* ability_ptr) {
  return nullptr;
}

class NoOpVSyncDispatcher final : public VSyncDispatcher {
 public:
  void RequestVSync() override { /* do nothing */
  }
};

VSyncDispatcher& VSyncDispatcher::Instance() {
  static base::NoDestructor<NoOpVSyncDispatcher> inst;
  return *inst;
}

}  // namespace animax
}  // namespace lynx
