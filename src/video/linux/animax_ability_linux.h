// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_LINUX_ANIMAX_ABILITY_LINUX_H_
#define ANIMAX_SRC_VIDEO_LINUX_ANIMAX_ABILITY_LINUX_H_

#include <utility>

#include "base/include/fml/task_runner.h"
#include "src/player/animax_ability.h"

namespace lynx {
namespace animax {

class AnimaXAbilityLinux : public AnimaXAbility {
 public:
  explicit AnimaXAbilityLinux(fml::RefPtr<fml::TaskRunner> raster_runner)
      : raster_runner_(std::move(raster_runner)) {}

  fml::RefPtr<fml::TaskRunner> GetRasterRunner() const {
    return raster_runner_;
  }

 private:
  fml::RefPtr<fml::TaskRunner> raster_runner_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_LINUX_ANIMAX_ABILITY_LINUX_H_
