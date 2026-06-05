// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_VK_CONTEXT_VK_H_
#define ANIMAX_SRC_RENDER_VK_CONTEXT_VK_H_

#include "skity/gpu/gpu_context.hpp"

namespace lynx {
namespace animax {

class ContextVk final {
 public:
  ContextVk() = delete;

  ~ContextVk() = delete;

  static std::shared_ptr<skity::GPUContext> GetGPUContext();
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_VK_CONTEXT_VK_H_
