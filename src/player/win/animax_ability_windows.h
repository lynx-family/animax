// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_WIN_ANIMAX_ABILITY_WINDOWS_H_
#define ANIMAX_SRC_PLAYER_WIN_ANIMAX_ABILITY_WINDOWS_H_

#include <memory>

#include "base/include/fml/task_runner.h"
#include "include/player/animax_player.h"
#include "src/base/log/log.h"
#include "src/player/animax_ability.h"

namespace lynx {
namespace animax {

class AnimaXAbilityWindows : public AnimaXAbility {
 public:
  explicit AnimaXAbilityWindows(fml::RefPtr<fml::TaskRunner> raster_runner)
      : raster_runner_(raster_runner) {
    DCHECK(raster_runner_);
  }

  fml::RefPtr<fml::TaskRunner> GetRasterRunner() const {
    return raster_runner_;
  }

  void SetAnimaXPlayer(std::weak_ptr<AnimaXPlayer> player) { player_ = player; }

  std::weak_ptr<AnimaXPlayer> GetAnimaXPlayer() const { return player_; }

 private:
  fml::RefPtr<fml::TaskRunner> raster_runner_;
  std::weak_ptr<AnimaXPlayer> player_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_WIN_ANIMAX_ABILITY_WINDOWS_H_
