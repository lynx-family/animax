// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/real_context.h"

#include "skity/gpu/gpu_context.hpp"

namespace lynx {
namespace animax {

ContextBackend RealContext::GetBackendType() const {
  switch (context_->GetBackendType()) {
    case skity::GPUBackendType::kOpenGL:
      return animax::ContextBackend::kOpenGL;
    case skity::GPUBackendType::kMetal:
      return animax::ContextBackend::kMetal;
    case skity::GPUBackendType::kVulkan:
      return animax::ContextBackend::kVulkan;
    default:
      return animax::ContextBackend::kOpenGL;
  }
}

}  // namespace animax
}  // namespace lynx
