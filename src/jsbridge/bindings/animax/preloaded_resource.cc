// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/jsbridge/bindings/animax/preloaded_resource.h"

#include <limits>
#include <mutex>
#include <utility>

#include "include/resource/bitmap.h"
#include "src/base/log/log.h"
#include "src/jsbridge/bindings/animax/napi_alpha_video.h"
#include "src/jsbridge/bindings/animax/napi_composition.h"
#include "src/jsbridge/bindings/animax/napi_font.h"
#include "src/jsbridge/bindings/animax/napi_image.h"
#include "src/jsbridge/bindings/animax/napi_preloaded_audio.h"
#include "src/jsbridge/bindings/animax/preload_host.h"
#include "src/model/composition_model.h"
#include "src/resource/asset/audio_asset.h"
#include "src/resource/asset/font_asset.h"
#include "src/resource/asset/video_asset.h"
#include "third_party/binding/common/object_ref.h"

#ifdef USE_PRIMJS_NAPI
#include "third_party/napi/include/primjs_napi_defines.h"
#endif

namespace lynx {
namespace animax {
namespace {

int64_t ImagePayloadBytes(const std::unique_ptr<Bitmap>& bitmap) {
  ANIMAX_CHECK(bitmap);
  // Bitmap owns its CPU pixels but only borrows texture handles.
  if (!bitmap->Pixels()) {
    return sizeof(Bitmap);
  }
  // PixelsLength() uses uint32_t. Widen before multiplying so images at or
  // above 4 GiB do not wrap, and keep the total within ImplBase's int64_t
  // range.
  const uint64_t pixel_count =
      static_cast<uint64_t>(bitmap->Width()) * bitmap->Height();
  const uint64_t bytes_per_pixel = bitmap->BytesPerPixel();
  constexpr uint64_t kMaxPixelBytes =
      std::numeric_limits<int64_t>::max() - sizeof(Bitmap);
  ANIMAX_CHECK(pixel_count <= kMaxPixelBytes / bytes_per_pixel);
  return static_cast<int64_t>(sizeof(Bitmap) + pixel_count * bytes_per_pixel);
}

}  // namespace

struct PreloadedResource::Storage {
  explicit Storage(int64_t bytes) : memory_usage(bytes) {
    ANIMAX_CHECK(bytes >= 0);
  }
  virtual ~Storage() = default;
  virtual void Release() = 0;

  // Serializes native transfer and release with dispose, including model
  // construction, memory recounting and old payload destruction. Synchronous
  // dispose and operation enqueueing may wait for this work. The lock is not
  // held while downloading, decoding or invoking JS callbacks.
  std::mutex mutex;
  PreloadedResourceState state = PreloadedResourceState::kAvailable;
  int64_t memory_usage;
  std::weak_ptr<PreloadHost> host;
};

struct Composition::Data : PreloadedResource::Storage {
  Data() : Storage(0) {}
  void Release() override {
    model.reset();
    config.reset();
    load_info = {};
  }
  std::shared_ptr<CompositionModel> model;
  std::shared_ptr<const AlphaPlayerConfig> config;
  CompositionLoadInfo load_info;
};

struct PreloadedImage::Data : PreloadedResource::Storage {
  explicit Data(std::unique_ptr<Bitmap> value)
      : Storage(ImagePayloadBytes(value)), bitmap(std::move(value)) {}
  void Release() override { bitmap.reset(); }
  std::unique_ptr<Bitmap> bitmap;
};

struct PreloadedAudio::Data : PreloadedResource::Storage {
  Data(std::shared_ptr<AudioAsset> value, int64_t bytes)
      : Storage(bytes), asset(std::move(value)) {
    ANIMAX_CHECK(asset && asset.use_count() == 1);
  }
  void Release() override { asset.reset(); }
  std::shared_ptr<AudioAsset> asset;
};

struct PreloadedFont::Data : PreloadedResource::Storage {
  Data(std::unique_ptr<RawData> value, std::string source, int64_t bytes)
      : Storage(bytes), data(std::move(value)), uri(std::move(source)) {}
  void Release() override {
    data.reset();
    std::string{}.swap(uri);
  }
  std::unique_ptr<RawData> data;
  std::string uri;
};

struct PreloadedAlphaVideo::Data : PreloadedResource::Storage {
  Data(std::shared_ptr<VideoAsset> value, int64_t bytes)
      : Storage(bytes), asset(std::move(value)) {
    ANIMAX_CHECK(asset && asset.use_count() == 1);
  }
  void Release() override { asset.reset(); }
  std::shared_ptr<VideoAsset> asset;
};

PreloadedResource::PreloadedResource(std::shared_ptr<Storage> storage)
    : storage_(std::move(storage)) {}

PreloadedResourceState PreloadedResource::GetState() const {
  std::lock_guard<std::mutex> lock(storage_->mutex);
  return storage_->state;
}

int64_t PreloadedResource::GetMemoryUsage() const {
  std::lock_guard<std::mutex> lock(storage_->mutex);
  return storage_->memory_usage;
}

std::shared_ptr<PreloadHost> PreloadedResource::GetHost() const {
  std::lock_guard<std::mutex> lock(storage_->mutex);
  return storage_->host.lock();
}

void PreloadedResource::SetHost(const std::shared_ptr<PreloadHost>& host) {
  std::lock_guard<std::mutex> lock(storage_->mutex);
  storage_->host = host;
}

void PreloadedResource::SyncExternalMemory() {
  OnExternalMemoryChanged(GetMemoryUsage());
}

void PreloadedResource::OnWrapped() { SyncExternalMemory(); }

void PreloadedResource::Dispose() {
  {
    std::lock_guard<std::mutex> lock(storage_->mutex);
    if (storage_->state == PreloadedResourceState::kDisposed) return;
    storage_->Release();
    storage_->state = PreloadedResourceState::kDisposed;
    storage_->memory_usage = 0;
  }
  SyncExternalMemory();
}

Composition::Composition(std::shared_ptr<CompositionModel> model,
                         CompositionLoadInfo load_info)
    : PreloadedResource(std::make_shared<Data>()) {
  ANIMAX_CHECK(model && model.use_count() == 1);
  auto data = std::static_pointer_cast<Data>(storage_);
  data->model = std::move(model);
  data->load_info = std::move(load_info);
}

Composition::Composition(std::shared_ptr<const AlphaPlayerConfig> config,
                         CompositionLoadInfo load_info)
    : PreloadedResource(std::make_shared<Data>()) {
  ANIMAX_CHECK(config && config.use_count() == 1);
  auto data = std::static_pointer_cast<Data>(storage_);
  data->config = std::move(config);
  data->load_info = std::move(load_info);
}

Composition::~Composition() = default;

CompositionModel* Composition::GetModel() const {
  return std::static_pointer_cast<Data>(storage_)->model.get();
}

const AlphaPlayerConfig* Composition::GetAlphaPlayerConfig() const {
  return std::static_pointer_cast<Data>(storage_)->config.get();
}

const CompositionLoadInfo& Composition::GetLoadInfo() const {
  return std::static_pointer_cast<Data>(storage_)->load_info;
}

PreloadedImage::PreloadedImage(std::unique_ptr<Bitmap> bitmap)
    : PreloadedResource(std::make_shared<Data>(std::move(bitmap))) {}
PreloadedImage::~PreloadedImage() = default;
const Bitmap* PreloadedImage::GetBitmap() const {
  return std::static_pointer_cast<Data>(storage_)->bitmap.get();
}

PreloadedAudio::PreloadedAudio(std::shared_ptr<AudioAsset> asset,
                               int64_t payload_bytes)
    : PreloadedResource(
          std::make_shared<Data>(std::move(asset), payload_bytes)) {}
PreloadedAudio::~PreloadedAudio() = default;
AudioAsset* PreloadedAudio::GetAsset() const {
  return std::static_pointer_cast<Data>(storage_)->asset.get();
}

PreloadedFont::PreloadedFont(std::unique_ptr<RawData> data, std::string uri,
                             int64_t payload_bytes)
    : PreloadedResource(std::make_shared<Data>(std::move(data), std::move(uri),
                                               payload_bytes)) {}
PreloadedFont::~PreloadedFont() = default;
const RawData* PreloadedFont::GetData() const {
  return std::static_pointer_cast<Data>(storage_)->data.get();
}

PreloadedAlphaVideo::PreloadedAlphaVideo(std::shared_ptr<VideoAsset> asset,
                                         int64_t payload_bytes)
    : PreloadedResource(
          std::make_shared<Data>(std::move(asset), payload_bytes)) {}
PreloadedAlphaVideo::~PreloadedAlphaVideo() = default;
VideoAsset* PreloadedAlphaVideo::GetAsset() const {
  return std::static_pointer_cast<Data>(storage_)->asset.get();
}

void InstallPreloadBindings(Napi::Env env, Napi::Object& module) {
  NapiComposition::Install(env, module);
  NapiImage::Install(env, module);
  NapiPreloadedAudio::Install(env, module);
  NapiFont::Install(env, module);
  NapiAlphaVideo::Install(env, module);
}

}  // namespace animax
}  // namespace lynx
