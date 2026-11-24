// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_BOUND_COMPOSITION_MEMORY_H_
#define ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_BOUND_COMPOSITION_MEMORY_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "src/jsbridge/bindings/animax/napi_task_runner.h"

namespace lynx {
namespace animax {

// One pending notification per bound composition. Close on the Env thread
// before releasing its JS accounting reference; resource threads only Post.
// The wrapper stays referenced until all tracked bytes are released or its Env
// exits. That reference does not extend the lifetime of the actual asset data.
class BoundMemoryDelivery
    : public std::enable_shared_from_this<BoundMemoryDelivery> {
 public:
  // Create with shared ownership (for example, make_shared). Otherwise posted
  // tasks never run and later Posts are ignored.
  explicit BoundMemoryDelivery(Napi::Env env);
  void Close();
  // At most one task is queued. A task posted while another is pending is
  // dropped rather than merged, so each task must read the current state when
  // it runs instead of carrying its own data.
  void Post(std::function<void()> task);
  bool IsOpen() const;

 private:
  mutable std::mutex mutex_;
  std::unique_ptr<NapiTaskRunner> runner_;
  bool pending_ = false;
};

class CompositionModel;

// Keeps only byte counts and a notification callback, never the model/assets.
// Data owners retain release tickets so an internal JS accounting reference
// cannot keep playback data alive or report it released before its last owner.
class BoundCompositionMemory
    : public std::enable_shared_from_this<BoundCompositionMemory> {
 public:
  using AssetBytes = std::unordered_map<std::string, int64_t>;
  using OnBytesChanged = std::function<void()>;

  // Prepare while the binding FIFO exclusively owns the model, then freeze
  // it on successful transfer. A failed attempt can close its notification
  // and later replace the tickets with a new record. The image and font
  // budgets come from the same prepared payloads as the model. The
  // initial total excludes source/configuration/failure records. The callback
  // runs on each releasing thread; callers must dispatch any NAPI work to its
  // owning Env and independently guard Env teardown.
  static std::shared_ptr<BoundCompositionMemory> Attach(
      CompositionModel& model, const AssetBytes& image_bytes,
      const AssetBytes& font_bytes, OnBytesChanged on_bytes_changed);

  int64_t GetBytes() const { return bytes_.load(std::memory_order_acquire); }

 private:
  struct ReleaseTicket;
  explicit BoundCompositionMemory(OnBytesChanged on_bytes_changed);
  std::shared_ptr<void> Track(int64_t bytes);
  void Release(int64_t bytes);

  std::atomic<int64_t> bytes_{0};
  const OnBytesChanged on_bytes_changed_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_BOUND_COMPOSITION_MEMORY_H_
