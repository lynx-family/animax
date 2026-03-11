// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_LAYER_EVENT_LISTENER_H_
#define ANIMAX_SRC_PLAYER_LAYER_EVENT_LISTENER_H_

#include <string>

#include "include/player/animax_event.h"

namespace lynx {
namespace animax {

class LayerEventListener {
 public:
  virtual ~LayerEventListener() = default;

  virtual void OnLayerError(const EventError err,
                            const std::string& err_msg) = 0;

  virtual void OnLayerWarning(const EventWarning warning,
                              const std::string& warning_msg) = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_LAYER_EVENT_LISTENER_H_
