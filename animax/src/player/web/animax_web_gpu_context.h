// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_WEB_ANIMAX_WEB_GPU_CONTEXT_H_
#define ANIMAX_SRC_PLAYER_WEB_ANIMAX_WEB_GPU_CONTEXT_H_

#include <webgpu/webgpu.h>

#include <memory>

namespace skity {
class GPUContext;
}

namespace lynx {
namespace animax {

class AnimaXWebGPUContext {
 public:
  AnimaXWebGPUContext();
  ~AnimaXWebGPUContext();

  skity::GPUContext* Context() const { return context_.get(); }

  WGPUDevice GetDevice() const { return device_; }
  WGPUQueue GetQueue() const { return queue_; }

  WGPUInstance GetInstance() const { return instance_; }

  static std::shared_ptr<AnimaXWebGPUContext> Create(uint64_t device_ptr,
                                                     uint64_t queue_ptr);

 private:
  WGPUInstance instance_;
  WGPUDevice device_;
  WGPUQueue queue_;
  std::unique_ptr<skity::GPUContext> context_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_WEB_ANIMAX_WEB_GPU_CONTEXT_H_
