// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_COMPOSITION_BINDING_STATE_H_
#define ANIMAX_SRC_PLAYER_COMPOSITION_BINDING_STATE_H_

#include <mutex>

#include "include/player/composition_binding.h"

namespace lynx {
namespace animax {
class PreloadHost;

// Lock order at installation: host validity, player state, composition data.
// No resource destruction, initialization or callback runs under these locks.
struct CompositionBindingState {
  mutable std::mutex mutex;
  bool valid = true;
  bool mode_locked = false;
  bool composition_mode = false;
  uint64_t latest_request_id = 0;
  uint64_t installed_id = 0;
  std::weak_ptr<PreloadHost> host;
  CompositionHostGuard host_guard;
  std::weak_ptr<CompositionBindingRequest> pending;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_COMPOSITION_BINDING_STATE_H_
