// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_HARMONY_FRAME_LISTENER_ADAPTER_H_
#define ANIMAX_SRC_BASE_UTIL_HARMONY_FRAME_LISTENER_ADAPTER_H_

#include <native_image/native_image.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace lynx {
namespace animax {

/**
 * Thread-safe context for frame available callback synchronization.
 */
struct FrameCallbackContext {
  std::mutex mutex;
  std::condition_variable cond;
  bool frame_available = false;
};

/**
 * Singleton adapter for managing OH_NativeImage frame available listeners.
 * Provides thread-safe callback handling to prevent dangling pointer access
 * when the VideoPlayerHarmony instance is destroyed.
 *
 * Usage:
 *   // Register:
 *   FrameListenerAdapter::GetInstance().Register(native_image, &context);
 *
 *   // Unregister (typically in destructor):
 *   FrameListenerAdapter::GetInstance().Unregister(native_image);
 */
class FrameListenerAdapter {
 public:
  static FrameListenerAdapter& GetInstance();

  /**
   * Register frame available listener for OH_NativeImage.
   * The context will be stored as weak_ptr for safe callback routing.
   */
  void Register(OH_NativeImage* native_image,
                std::weak_ptr<FrameCallbackContext> context);

  /**
   * Unregister frame available listener.
   * After this call, any pending callbacks will be safely ignored.
   */
  void Unregister(OH_NativeImage* native_image);

  /**
   * Called by the static callback function to notify frame available.
   * Thread-safe: checks if native_image is still registered before signaling.
   */
  void OnFrameAvailable(OH_NativeImage* native_image);

  /**
   * Wait for frame available callback with timeout.
   * Directly uses the provided context without map lookup.
   */
  static bool WaitForFrameAvailable(FrameCallbackContext& context,
                                    std::chrono::milliseconds timeout);

 private:
  FrameListenerAdapter() = default;

  std::mutex mutex_;
  std::unordered_map<OH_NativeImage*, std::weak_ptr<FrameCallbackContext>>
      contexts_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_HARMONY_FRAME_LISTENER_ADAPTER_H_
