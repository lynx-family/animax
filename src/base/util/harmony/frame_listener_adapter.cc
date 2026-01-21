// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/harmony/frame_listener_adapter.h"

namespace lynx {
namespace animax {

namespace {
void OnFrameAvailableCB(void* context) {
  auto* native_image = static_cast<OH_NativeImage*>(context);
  FrameListenerAdapter::GetInstance().OnFrameAvailable(native_image);
}
}  // namespace

FrameListenerAdapter& FrameListenerAdapter::GetInstance() {
  static FrameListenerAdapter instance;
  return instance;
}

void FrameListenerAdapter::Register(
    OH_NativeImage* native_image, std::weak_ptr<FrameCallbackContext> context) {
  if (native_image == nullptr) {
    return;
  }

  {
    std::unique_lock<std::mutex> lock(mutex_);
    contexts_[native_image] = std::move(context);
  }

  // Register callback with native_image as context
  OH_OnFrameAvailableListener listener = {native_image, OnFrameAvailableCB};
  OH_NativeImage_SetOnFrameAvailableListener(native_image, listener);
}

void FrameListenerAdapter::Unregister(OH_NativeImage* native_image) {
  if (native_image == nullptr) {
    return;
  }

  {
    std::unique_lock<std::mutex> lock(mutex_);
    contexts_.erase(native_image);
  }

  // Unset listener first to prevent new callbacks
  OH_NativeImage_UnsetOnFrameAvailableListener(native_image);
}

void FrameListenerAdapter::OnFrameAvailable(OH_NativeImage* native_image) {
  if (native_image == nullptr) {
    return;
  }

  std::shared_ptr<FrameCallbackContext> context;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = contexts_.find(native_image);
    if (it == contexts_.end()) {
      return;
    }
    context = it->second.lock();
  }

  // Signal the context outside adapter lock using shared_ptr.
  if (context) {
    std::unique_lock<std::mutex> ctx_lock(context->mutex);
    context->frame_available = true;
    context->cond.notify_all();
  }
}

bool FrameListenerAdapter::WaitForFrameAvailable(
    FrameCallbackContext& context, std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(context.mutex);
  if (context.frame_available) {
    context.frame_available = false;
    return true;
  }

  bool available = context.cond.wait_for(
      lock, timeout, [&context]() { return context.frame_available; });

  context.frame_available = false;
  return available;
}

}  // namespace animax
}  // namespace lynx
