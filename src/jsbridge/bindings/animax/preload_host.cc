// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/jsbridge/bindings/animax/preload_host.h"

#include <cstdint>
#include <utility>

namespace lynx {
namespace animax {
namespace {

// FNV-1a("lynx.animax.PreloadHost") keeps the slot stable across modules.
// An address-based key could select different slots in separate libraries.
constexpr uint64_t kPreloadHostDataKey = 0xeb25a692dc9e8fe3;
constexpr const char* kHostUnavailable = "Preload host is unavailable";

struct PreloadHostData {
  std::shared_ptr<PreloadHost> host;

  ~PreloadHostData() {
    // Fallback if the platform did not deliver its explicit detach callback.
    if (host) {
      host->Invalidate();
    }
  }
};

}  // namespace

PreloadHost::PreloadHost(Provider provider) : provider_(std::move(provider)) {}

void PreloadHost::GetCapabilities(Callback callback) {
  if (!IsValid() || !provider_) {
    callback({}, kHostUnavailable);
    return;
  }
  provider_([weak_host = weak_from_this(), callback = std::move(callback)](
                PreloadHostCapabilities capabilities, std::string error) {
    // The page may exit while the provider is waiting on its platform thread.
    // Discard that snapshot, but still complete the native caller for cleanup.
    auto host = weak_host.lock();
    if (!host || !host->IsValid()) {
      callback({}, kHostUnavailable);
      return;
    }
    callback(std::move(capabilities), std::move(error));
  });
}

void PreloadHost::Invalidate() {
  std::lock_guard<std::mutex> lock(validity_mutex_);
  valid_.store(false, std::memory_order_release);
}

bool PreloadHost::RunIfValid(const std::function<void()>& callback) {
  std::lock_guard<std::mutex> lock(validity_mutex_);
  if (!IsValid()) return false;
  callback();
  return true;
}

bool PreloadHost::IsValid() const {
  return valid_.load(std::memory_order_acquire);
}

bool PreloadHost::AttachToEnv(Napi::Env env,
                              std::shared_ptr<PreloadHost> host) {
  if (!host || !host->IsValid()) {
    return false;
  }
  auto* data = env.GetInstanceData<PreloadHostData>(kPreloadHostDataKey);
  if (data) {
    return data->host == host;
  }
  env.SetInstanceData(kPreloadHostDataKey,
                      new PreloadHostData{std::move(host)});
  return true;
}

std::shared_ptr<PreloadHost> PreloadHost::FromEnv(Napi::Env env) {
  auto* data = env.GetInstanceData<PreloadHostData>(kPreloadHostDataKey);
  return data ? data->host : nullptr;
}

void PreloadHost::DetachFromEnv(Napi::Env env) {
  auto* data = env.GetInstanceData<PreloadHostData>(kPreloadHostDataKey);
  if (!data) {
    env.SetInstanceData(kPreloadHostDataKey, new PreloadHostData{});
  } else if (data->host) {
    data->host->Invalidate();
    data->host.reset();
  }
}

}  // namespace animax
}  // namespace lynx
