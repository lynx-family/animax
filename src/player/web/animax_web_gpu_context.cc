// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/web/animax_web_gpu_context.h"

#include "skity/gpu/gpu_context_web.hpp"

namespace lynx {
namespace animax {

AnimaXWebGPUContext::AnimaXWebGPUContext() = default;

AnimaXWebGPUContext::~AnimaXWebGPUContext() {
  wgpuQueueRelease(queue_);
  wgpuDeviceRelease(device_);
  wgpuInstanceRelease(instance_);
}

std::shared_ptr<AnimaXWebGPUContext> AnimaXWebGPUContext::Create(
    uint64_t device_ptr, uint64_t queue_ptr) {
  auto device = reinterpret_cast<WGPUDevice>(device_ptr);
  auto queue = reinterpret_cast<WGPUQueue>(queue_ptr);

  auto skity_context = skity::WebContextCreate(device, queue);

  if (!skity_context) {
    return {};
  }

  auto context = new AnimaXWebGPUContext();
  context->device_ = device;
  context->queue_ = queue;
  context->context_ = std::move(skity_context);
  context->instance_ = wgpuCreateInstance(nullptr);

  return std::shared_ptr<AnimaXWebGPUContext>{context};
}

}  // namespace animax
}  // namespace lynx
