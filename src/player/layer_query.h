// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_LAYER_QUERY_H_
#define ANIMAX_SRC_PLAYER_LAYER_QUERY_H_

namespace lynx {
namespace animax {

enum class LayerQueryType { kBounds, kPosition, kUnsupported };

struct LayerPosition {
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
};

struct LayerQueryResult {
  bool success = false;
  LayerPosition position;
  float width = 0.f;
  float height = 0.f;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_LAYER_QUERY_H_
