// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/property/animax_player_global.h"

#include "include/player/animax_player.h"
#include "src/base/thread/thread_assert.h"

namespace lynx {
namespace animax {
namespace {

AnimaXPlayerGlobal& AnimaXPLayerGlobalInstance() {
  static base::NoDestructor<AnimaXPlayerGlobal> instance{};
  return *instance;
}

std::mutex& AnimaXPlayerGlobalInstanceMutex() {
  static base::NoDestructor<std::mutex> mutex{};
  return *mutex;
}
}  // namespace

std::weak_ptr<AnimaXPlayer> AnimaXPlayerGlobal::Find(const std::string& id) {
  auto weak_player = std::weak_ptr<AnimaXPlayer>{};
  {
    const auto lock_guard =
        std::lock_guard<std::mutex>(AnimaXPlayerGlobalInstanceMutex());
    auto& global_player_map = AnimaXPLayerGlobalInstance().player_map_;
    if (auto it = global_player_map.find(id); it != global_player_map.end()) {
      weak_player = it->second;
    }
  }
  return weak_player;
}

void AnimaXPlayerGlobal::Register(const std::string& id,
                                  std::weak_ptr<AnimaXPlayer> player) {
  const auto lock_guard =
      std::lock_guard<std::mutex>(AnimaXPlayerGlobalInstanceMutex());
  AnimaXPLayerGlobalInstance().player_map_[id] = std::move(player);
}

void AnimaXPlayerGlobal::Unregister(const std::string& id) {
  const auto lock_guard =
      std::lock_guard<std::mutex>(AnimaXPlayerGlobalInstanceMutex());
  AnimaXPLayerGlobalInstance().player_map_.erase(id);
}

}  // namespace animax
}  // namespace lynx
