// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/jsbridge/bindings/animax/bound_composition_memory.h"

#include <utility>

#include "src/jsbridge/bindings/animax/preload_model_memory.h"
#include "src/model/composition_model.h"

namespace lynx {
namespace animax {

BoundMemoryDelivery::BoundMemoryDelivery(Napi::Env env)
    : runner_(std::make_unique<NapiTaskRunner>(env)) {}

void BoundMemoryDelivery::Close() {
  std::lock_guard<std::mutex> lock(mutex_);
  runner_.reset();
}

bool BoundMemoryDelivery::IsOpen() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return runner_ != nullptr;
}

void BoundMemoryDelivery::Post(std::function<void()> task) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!runner_ || pending_) return;
  pending_ = true;
  if (!runner_->PostTask([weak = weak_from_this(), task = std::move(task)] {
        auto self = weak.lock();
        if (!self) return;
        {
          std::lock_guard<std::mutex> lock(self->mutex_);
          if (!self->runner_) return;
          // The task reads the current total. Releases after this reset
          // schedule another notification; releases before it share this
          // pending task.
          self->pending_ = false;
        }
        task();
      })) {
    pending_ = false;
  }
}

struct BoundCompositionMemory::ReleaseTicket {
  ReleaseTicket(std::shared_ptr<BoundCompositionMemory> owner, int64_t bytes)
      : owner(std::move(owner)), bytes(bytes) {}
  ~ReleaseTicket() { owner->Release(bytes); }

  std::shared_ptr<BoundCompositionMemory> owner;
  const int64_t bytes;
};

BoundCompositionMemory::BoundCompositionMemory(OnBytesChanged on_bytes_changed)
    : on_bytes_changed_(std::move(on_bytes_changed)) {}

std::shared_ptr<void> BoundCompositionMemory::Track(int64_t bytes) {
  if (bytes == 0) return nullptr;
  bytes_.fetch_add(bytes, std::memory_order_relaxed);
  return std::make_shared<ReleaseTicket>(shared_from_this(), bytes);
}

void BoundCompositionMemory::Release(int64_t bytes) {
  bytes_.fetch_sub(bytes, std::memory_order_acq_rel);
  if (on_bytes_changed_) on_bytes_changed_();
}

std::shared_ptr<BoundCompositionMemory> BoundCompositionMemory::Attach(
    CompositionModel& model, const AssetBytes& image_bytes,
    const AssetBytes& font_bytes, OnBytesChanged on_bytes_changed) {
  auto memory = std::shared_ptr<BoundCompositionMemory>(
      new BoundCompositionMemory(std::move(on_bytes_changed)));
  int64_t model_bytes = EstimatePreloadedModelBytes(model);
  const auto attach_asset = [&memory, &model_bytes](const auto& asset) {
    if (!asset) return;
    const int64_t bytes = EstimatePreloadedAssetBytes(*asset);
    model_bytes -= bytes;
    asset->bound_memory_ticket_ = memory->Track(bytes);
  };
  for (const auto& [id, asset] : model.GetImages()) {
    if (!asset) continue;
    const int64_t bytes = EstimatePreloadedAssetBytes(*asset);
    const int64_t file_name_bytes = EstimatePreloadedImageFileNameBytes(*asset);
    model_bytes -= bytes;
    asset->bound_memory_ticket_ = memory->Track(bytes - file_name_bytes);
    // GetImage clears the potentially large base64 source name independently
    // of the ImageAsset and Bitmap lifetimes.
    asset->bound_file_name_memory_ticket_ = memory->Track(file_name_bytes);
    const auto entry = image_bytes.find(id);
    if (asset->bitmap_ && entry != image_bytes.end()) {
      asset->bitmap_->bound_memory_ticket_ = memory->Track(entry->second);
    }
  }
  for (const auto& [id, asset] : model.GetFonts()) {
    if (!asset) continue;
    const int64_t bytes = EstimatePreloadedAssetBytes(*asset);
    model_bytes -= bytes;
    asset->bound_memory_ticket_ = memory->Track(bytes);
    // SetRawData and ResetModel release the RawData independently of the
    // FontAsset lifetime.
    const auto entry = font_bytes.find(id);
    asset->bound_raw_data_memory_ticket_ =
        memory->Track(entry == font_bytes.end() ? 0 : entry->second);
  }
  for (const auto& [id, asset] : model.GetAudios()) attach_asset(asset);
  for (const auto& [id, asset] : model.GetVideos()) attach_asset(asset);
  model.bound_memory_ticket_ = memory->Track(model_bytes);
  return memory;
}

}  // namespace animax
}  // namespace lynx
