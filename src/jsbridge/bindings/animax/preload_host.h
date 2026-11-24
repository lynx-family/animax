// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOAD_HOST_H_
#define ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOAD_HOST_H_

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "include/resource/unzip_task.h"
#include "third_party/binding/napi/shim/shim_napi.h"

namespace lynx {
namespace animax {

// Per-acquisition access to existing loaders; no resource is loaded here.
// The shared pointers retain the loaders, not the page or its NAPI Env.
// New preload callers initiate Load on Resource. Platform callbacks may run
// on any thread; consumers dispatch preparation and JS delivery separately.
struct PreloadHostCapabilities {
  Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader;
  Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader;
  // Sampled from the same host metrics used by Player. Later metric changes
  // must not change the scale already captured for a request.
  float scale = 0;
};

// Connects one page's NAPI Env to its platform resource services without
// requiring an animation component. Platform adapters own service creation.
class PreloadHost : public std::enable_shared_from_this<PreloadHost> {
 public:
  // Native-only completion. An empty error means success; on failure the
  // provider returns empty capabilities and a non-empty error.
  using Callback =
      std::function<void(PreloadHostCapabilities, std::string error)>;
  // Invoked on GetCapabilities' calling thread. Dispatch to the platform's
  // required thread, validate the services/scale, and complete exactly once.
  using Provider = std::function<void(Callback)>;

  // Create with shared ownership (for example, make_shared): GetCapabilities
  // checks lifetime through weak_from_this. The provider must not retain the
  // page.
  explicit PreloadHost(Provider provider);

  // Obtain loader handles and a fresh scale snapshot, without starting a load.
  // This class does not dispatch threads. Early failures complete inline;
  // otherwise completion runs on the provider's chosen thread, not necessarily
  // the JS thread. Recheck host and Env validity before publishing JS results.
  void GetCapabilities(Callback callback);

  // Idempotently mark this host unusable from any thread. This does not cancel
  // work, clear the Env slot, or release loader references held elsewhere.
  // The platform adapter separately releases its own loaders on page exit.
  void Invalidate();

  // Read the lifetime flag only; this neither validates services nor keeps
  // the Env alive. A true result is not a lifetime guarantee for later work.
  bool IsValid() const;

  // Serialize the final pointer handoff with page invalidation. The callback
  // must be short and synchronous: no loading, thread dispatch or JS calls.
  bool RunIfValid(const std::function<void()>& callback);

  // The following Env operations require a live Env and its JS thread.
  // Store one strong host reference in existing Env instance data. Returns true
  // for the first attach or the same valid host; rejects replacement or an Env
  // already detached through DetachFromEnv.
  static bool AttachToEnv(Napi::Env env, std::shared_ptr<PreloadHost> host);

  // Return nullptr when absent/detached. A returned host may already be
  // invalid. Call GetCapabilities or IsValid; lookup alone does not prove the
  // page is usable.
  static std::shared_ptr<PreloadHost> FromEnv(Napi::Env env);

  // Invalidate and drop the Env's host reference. Repeated calls are harmless.
  // Keep an empty slot to reject late attach; Env cleanup deletes the holder.
  static void DetachFromEnv(Napi::Env env);

 private:
  const Provider provider_;
  std::mutex validity_mutex_;
  std::atomic<bool> valid_{true};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOAD_HOST_H_
