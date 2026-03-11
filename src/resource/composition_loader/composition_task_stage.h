// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_TASK_STAGE_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_TASK_STAGE_H_

namespace lynx {
namespace animax {

enum class ParseStage {
  kRequestStart = 0,       // Request src start
  kRequestEnd,             // Request src success
  kParseCompositionStart,  // Parse composition start
  kParseCompositionEnd,    // Parse composition end
  kLoadAssetStart,         // Load image/font start
  kLoadAssetEnd,           // Load image/font end
  kBuildLayerStart,        // Build composition layer start
  kBuildLayerEnd,          // Build composition layer end
  kAnimationReady,         // Ready to play animation
};

}
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_TASK_STAGE_H_
