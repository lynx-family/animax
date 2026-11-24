// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/jsbridge/bindings/animax/preloaded_resource.h"

#include <algorithm>
#include <deque>
#include <limits>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "include/resource/bitmap.h"
#include "src/base/log/log.h"
#include "src/base/thread/task_runner.h"
#include "src/jsbridge/bindings/animax/napi_alpha_video.h"
#include "src/jsbridge/bindings/animax/napi_composition.h"
#include "src/jsbridge/bindings/animax/napi_font.h"
#include "src/jsbridge/bindings/animax/napi_image.h"
#include "src/jsbridge/bindings/animax/napi_on_resource_update_callback.h"
#include "src/jsbridge/bindings/animax/napi_preloaded_audio.h"
#include "src/jsbridge/bindings/animax/napi_task_runner.h"
#include "src/jsbridge/bindings/animax/preload_host.h"
#include "src/model/composition_model.h"
#include "src/model/layer_model.h"
#include "src/parser/alpha_player_asset/alpha_player_asset_parser.h"
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
    pending_audio.reset();
    image_bytes.clear();
    font_bytes.clear();
    load_info = {};
  }
  // Counts prepared payloads only; the model structure is not estimated.
  // ImageAsset keeps Bitmap private, so transferred bitmap and font bytes are
  // tracked by slot.
  void Recount() {
    int64_t bytes = pending_audio ? pending_audio->GetMemoryUsageBytes() : 0;
    if (model) {
      for (const auto& entry : model->GetAudios()) {
        if (entry.second) bytes += entry.second->GetMemoryUsageBytes();
      }
      for (const auto& entry : model->GetVideos()) {
        if (entry.second) bytes += entry.second->GetMemoryUsageBytes();
      }
    }
    for (const auto& entry : image_bytes) bytes += entry.second;
    for (const auto& entry : font_bytes) bytes += entry.second;
    memory_usage = bytes;
  }

  std::shared_ptr<CompositionModel> model;
  std::shared_ptr<const AlphaPlayerConfig> config;
  std::shared_ptr<AudioAsset> pending_audio;
  CompositionLoadInfo load_info;
  std::unordered_map<std::string, int64_t> image_bytes;
  std::unordered_map<std::string, int64_t> font_bytes;
  std::deque<std::shared_ptr<CompositionOperation>> updates;
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

namespace {
constexpr uint64_t kUpdateReferences = 0xb23db5d87f041a69;

// Owned and destroyed by the Env on its JS thread. Resource tasks never own
// ObjectRef or borrowed Impl pointers, including when Env teardown wins.
struct UpdateReferences {
  struct Entry {
    Composition* composition;
    PreloadedResource* resource;
    binding::ObjectRef composition_ref;
    binding::ObjectRef resource_ref;
  };
  std::unordered_map<uintptr_t, Entry> entries;
};

const char* UnavailableCode(PreloadedResourceState state) {
  return state == PreloadedResourceState::kDisposed ? "Destroyed" : "Consumed";
}
}  // namespace

class CompositionOperation
    : public std::enable_shared_from_this<CompositionOperation> {
 public:
  CompositionOperation(std::shared_ptr<Composition::Data> composition,
                       std::shared_ptr<PreloadedResource::Storage> resource,
                       ResourceType type, std::string id,
                       std::unique_ptr<NapiOnResourceUpdateCallback> callback,
                       Napi::Env env)
      : composition_(composition),
        resource_(std::move(resource)),
        type_(type),
        id_(std::move(id)),
        callback_(std::move(callback)),
        runner_(env),
        host_(composition->host) {}

  CompositionOperation(std::shared_ptr<Composition::Data> composition,
                       std::string name, std::string value,
                       std::unique_ptr<NapiOnResourceUpdateCallback> callback,
                       Napi::Env env)
      : composition_(composition),
        type_(ResourceType::kUnknown),
        id_(std::move(name)),
        string_value_(std::move(value)),
        callback_(std::move(callback)),
        runner_(env),
        host_(composition->host) {}

  void Start() {
    GetAnimaXResourceThread()->PostTask(
        [self = shared_from_this()] { self->Run(); });
  }

  void Cancel() {
    auto composition = composition_.lock();
    if (!composition) return;
    {
      std::lock_guard<std::mutex> lock(composition->mutex);
      if (!IsPending()) return;
      phase_ = Phase::kFailed;
    }
    Complete("Destroyed",
             "Composition was disposed before the operation completed");
  }

 private:
  enum class Phase { kPending, kCommitted, kFailed };
  bool IsPending() const { return phase_ == Phase::kPending; }

  // The FIFO owns the unbound model and its storage mutex is held. Collect
  // before writing so one malformed matching layer cannot leave a partial edit.
  const char* UpdateText(Composition::Data& composition, std::string& message) {
    if (id_.find_first_not_of(" \t\r\n") == std::string::npos) {
      message = "Expected a nonempty text layer name";
      return "InvalidArgument";
    }
    auto owner = host_.lock();
    if (!owner || !owner->IsValid()) {
      message = "The composition host is unavailable";
      return "EnvironmentUnavailable";
    }
    if (composition.state != PreloadedResourceState::kAvailable) {
      message = "Composition is no longer editable";
      return UnavailableCode(composition.state);
    }
    std::vector<DocumentData*> documents;
    if (auto* model = composition.model.get()) {
      std::vector<LayerModelList*> pending{&model->GetLayers()};
      std::unordered_set<std::string> visited;
      while (!pending.empty()) {
        auto* layers = pending.back();
        pending.pop_back();
        for (const auto& layer : *layers) {
          if (!layer) continue;
          if (layer->GetLayerType() == LayerType::kPreComp) {
            const auto& id = layer->GetRefId();
            auto it = model->GetPrecomps().find(id);
            if (it != model->GetPrecomps().end() && visited.insert(id).second) {
              pending.push_back(&it->second);
            }
          } else if (layer->GetLayerType() == LayerType::kText &&
                     layer->GetName() == id_) {
            auto* text = layer->GetText();
            if (!text || text->GetKeyframes().empty()) {
              message = "A matching text layer has no editable text data";
              return "PreparationFailed";
            }
            for (const auto& frame : text->GetKeyframes()) {
              auto* start = frame ? frame->GetStartValue<Value>() : nullptr;
              auto* end = frame ? frame->GetEndValue<Value>() : nullptr;
              if (!start || start->GetValueType() != ValueType::kDocument ||
                  (end && end->GetValueType() != ValueType::kDocument)) {
                message = "A matching text layer has invalid text keyframes";
                return "PreparationFailed";
              }
              documents.push_back(static_cast<DocumentData*>(start));
              // A missing end value is valid: text animation uses the start.
              if (end) documents.push_back(static_cast<DocumentData*>(end));
            }
          }
        }
      }
    }
    if (documents.empty()) {
      message = "No used text layer matches the requested name";
      return "ResourceNotFound";
    }
    for (auto* document : documents) document->SetText(*string_value_);
    composition.Recount();
    return nullptr;
  }

  // Both native storage locks are held. This is also the cross-Composition
  // claim: a resource cannot be disposed or consumed by another transfer
  // between validation and commit. Waiting in the FIFO does not claim it.
  const char* Transfer(Composition::Data& composition, std::string& message) {
    if (id_.find_first_not_of(" \t\r\n") == std::string::npos) {
      message = "Expected a nonempty resource ID";
      return "InvalidArgument";
    }
    auto owner = host_.lock();
    if (!owner || !owner->IsValid()) {
      message = "The composition host is unavailable";
      return "EnvironmentUnavailable";
    }
    if (resource_->host.lock() != owner) {
      message = "Resource and composition belong to different hosts";
      return "HostMismatch";
    }
    if (composition.state != PreloadedResourceState::kAvailable) {
      message = "Composition is no longer editable";
      return UnavailableCode(composition.state);
    }
    if (resource_->state != PreloadedResourceState::kAvailable) {
      message = "Resource is no longer available";
      return UnavailableCode(resource_->state);
    }

    auto* model = composition.model.get();
    if (type_ == ResourceType::kImage) {
      auto image = std::static_pointer_cast<PreloadedImage::Data>(resource_);
      if (!model || model->GetImages().find(id_) == model->GetImages().end() ||
          !model->GetImages().at(id_))
        return MissingSlot(composition, message);
      if (!image->bitmap) return InvalidData(message);
      composition.image_bytes[id_] = resource_->memory_usage;
      model->GetImages().at(id_)->SetBitmap(std::move(image->bitmap));
    } else if (type_ == ResourceType::kFont) {
      auto font = std::static_pointer_cast<PreloadedFont::Data>(resource_);
      if (!model || model->GetFonts().find(id_) == model->GetFonts().end() ||
          !model->GetFonts().at(id_))
        return MissingSlot(composition, message);
      if (!font->data) return InvalidData(message);
      auto metadata = model->GetFonts().at(id_)->Model();
      metadata.path = font->uri;
      metadata.path_origin = FontPathOrigin::kFontUrl;
      // A fresh asset discards a previously parsed Font without changing the
      // old asset before all validation succeeds. SetRawData alone cannot do
      // it.
      auto replacement = std::make_shared<FontAsset>(std::move(metadata));
      composition.font_bytes[id_] = resource_->memory_usage;
      replacement->SetRawData(std::move(font->data));
      model->GetFonts().at(id_) = std::move(replacement);
      std::string{}.swap(font->uri);
    } else if (type_ == ResourceType::kAudio) {
      auto audio = std::static_pointer_cast<PreloadedAudio::Data>(resource_);
      const bool alpha_audio = composition.config && id_ == "1" &&
                               composition.load_info.enable_audio;
      if (!alpha_audio &&
          (!model || model->GetAudios().find(id_) == model->GetAudios().end() ||
           !model->GetAudios().at(id_)))
        return MissingSlot(composition, message);
      if (!audio->asset || !audio->asset->IsValid())
        return InvalidData(message);
      // Keep the prepared resource's actual source; only its slot ID changes.
      auto metadata = audio->asset->Model();
      metadata.id = id_;
      audio->asset->ResetModel(std::move(metadata));
      if (model)
        model->GetAudios().at(id_) = std::move(audio->asset);
      else
        composition.pending_audio = std::move(audio->asset);
    } else {
      auto video =
          std::static_pointer_cast<PreloadedAlphaVideo::Data>(resource_);
      const bool alpha_video = composition.config && id_ == "0";
      if (!alpha_video &&
          (!model || model->GetVideos().find(id_) == model->GetVideos().end() ||
           !model->GetVideos().at(id_)))
        return MissingSlot(composition, message);
      if (!video->asset) return InvalidData(message);
      const char* code = InstallVideo(composition, id_, video->asset, message);
      if (code) return code;
    }
    resource_->state = PreloadedResourceState::kConsumed;
    resource_->memory_usage = 0;
    composition.Recount();
    return nullptr;
  }

  // Validation and AlphaPlayer construction use candidates; existing data
  // changes only after
  // both succeed. The caller holds the Composition storage lock.
  static const char* InstallVideo(Composition::Data& composition,
                                  const std::string& id,
                                  std::shared_ptr<VideoAsset>& asset,
                                  std::string& message) {
    auto* model = composition.model.get();
    const bool alpha_video = composition.config && id == "0";
    auto metadata = asset->Model();
    metadata.id = id;
    if (alpha_video) {
      metadata.rgb_frame = composition.config->rgb_frame;
      metadata.a_frame = composition.config->alpha_frame;
    } else {
      metadata.rgb_frame = model->GetVideos().at(id)->Model().rgb_frame;
      metadata.a_frame = model->GetVideos().at(id)->Model().a_frame;
    }
    auto error = asset->ResolvePreparedModel(&metadata);
    if (error) {
      message = std::move(error.message);
      return "InvalidVideoRegion";
    }
    std::shared_ptr<CompositionModel> replacement;
    if (alpha_video) {
      auto audio = composition.pending_audio;
      if (model) {
        auto it = model->GetAudios().find("1");
        if (it != model->GetAudios().end()) audio = it->second;
      }
      replacement = AlphaPlayerAssetParser::BuildFromPreparedVideo(
          *composition.config, asset, metadata, audio,
          composition.load_info.scale, composition.load_info.enable_audio);
      if (!replacement) {
        message = "Could not construct the AlphaPlayer composition";
        return "PreparationFailed";
      }
    }
    // All fallible preparation above used local candidates. Adapt only at
    // the commit point; no read, decode or timeline change for full JSON.
    error = asset->SetPreparedModel(std::move(metadata));
    if (error) {
      message = std::move(error.message);
      return "InvalidVideoRegion";
    }
    if (replacement) {
      composition.model = std::move(replacement);
      composition.pending_audio.reset();
      asset.reset();
    } else {
      model->GetVideos().at(id) = std::move(asset);
    }
    return nullptr;
  }

  const char* MissingSlot(const Composition::Data& composition,
                          std::string& message) const {
    auto* model = composition.model.get();
    const bool other_type =
        model &&
        (model->GetImages().count(id_) || model->GetAudios().count(id_) ||
         model->GetVideos().count(id_) || model->GetFonts().count(id_));
    message = "No matching resource slot: " + id_;
    return other_type ? "ResourceTypeMismatch" : "ResourceNotFound";
  }

  static const char* InvalidData(std::string& message) {
    message = "Prepared resource data is unavailable";
    return "PreparationFailed";
  }

  void Run() {
    auto composition = composition_.lock();
    if (!composition) return;
    std::string message;
    const char* code = nullptr;
    if (string_value_) {
      std::lock_guard<std::mutex> lock(composition->mutex);
      if (!IsPending()) return;
      code = UpdateText(*composition, message);
      phase_ = code ? Phase::kFailed : Phase::kCommitted;
    } else if (!resource_) {
      // Ownership was rejected before any source JS reference was acquired.
      // Keep the failed request in the FIFO without touching foreign storage.
      std::lock_guard<std::mutex> lock(composition->mutex);
      if (!IsPending()) return;
      code = "HostMismatch";
      message =
          "Resource and composition belong to different environments or hosts";
      phase_ = Phase::kFailed;
    } else {
      std::scoped_lock lock(composition->mutex, resource_->mutex);
      if (!IsPending()) return;
      code = Transfer(*composition, message);
      phase_ = code ? Phase::kFailed : Phase::kCommitted;
    }
    Complete(code ? code : "", std::move(message));
  }

  void Complete(std::string code, std::string message) {
    // Run and Cancel choose the result under the same storage lock. Only the
    // operation that changes Pending to a final state can complete this call.
    runner_.PostTask([self = shared_from_this(), code = std::move(code),
                      message = std::move(message)] {
      bool valid = false;
      auto env = self->callback_->Env(&valid);
      if (!valid) return;
      Napi::ContextScope context_scope(env);
      Napi::HandleScope handle_scope(env);
      auto* references =
          env.GetInstanceData<UpdateReferences>(kUpdateReferences);
      if (!references) return;
      auto it =
          references->entries.find(reinterpret_cast<uintptr_t>(self.get()));
      if (it == references->entries.end()) return;
      auto refs = std::move(it->second);
      references->entries.erase(it);
      // Synchronize current totals, not captured deltas: dispose or a later
      // native release must never be undone by a delayed JS notification.
      if (refs.resource) refs.resource->SyncExternalMemory();
      refs.composition->SyncExternalMemory();
      auto owner = self->host_.lock();
      if (owner && owner->IsValid() && PreloadHost::FromEnv(env) == owner) {
        self->callback_->Invoke(code.empty(), code, message);
      } else {
        // Generated callbacks pop their holder only on Invoke. Page exit must
        // release it without invoking JavaScript in the departed page.
        auto* storage = env.GetInstanceData<binding::HolderStorage>(
            kNapiOnResourceUpdateCallbackClassID);
        auto key = reinterpret_cast<uintptr_t>(self->callback_.get());
        storage->PopHolder(key);
      }
      self->Advance();
    });
  }

  void Advance() {
    auto composition = composition_.lock();
    if (!composition) return;
    std::shared_ptr<CompositionOperation> next;
    {
      std::lock_guard<std::mutex> lock(composition->mutex);
      auto& queue = composition->updates;
      auto it = std::find_if(
          queue.begin(), queue.end(),
          [this](const auto& value) { return value.get() == this; });
      if (it == queue.end()) return;
      const bool was_first = it == queue.begin();
      queue.erase(it);
      if (was_first && !queue.empty() &&
          composition->state != PreloadedResourceState::kDisposed) {
        next = queue.front();
      }
    }
    if (next) next->Start();
  }

  std::weak_ptr<Composition::Data> composition_;
  // Null for string updates or an injection Env/host mismatch.
  // Such an entry must not extend the source resource's lifetime or access its
  // Env on completion.
  std::shared_ptr<PreloadedResource::Storage> resource_;
  ResourceType type_;
  std::string id_;
  // Empty text is meaningful and must remain distinct from an operation
  // without a string.
  std::optional<std::string> string_value_;
  std::unique_ptr<NapiOnResourceUpdateCallback> callback_;
  NapiTaskRunner runner_;
  std::weak_ptr<PreloadHost> host_;
  // Guarded by the composition storage mutex.
  Phase phase_ = Phase::kPending;
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

void Composition::Dispose() {
  PreloadedResource::Dispose();
  auto data = std::static_pointer_cast<Data>(storage_);
  std::deque<std::shared_ptr<CompositionOperation>> updates;
  {
    std::lock_guard<std::mutex> lock(data->mutex);
    updates = data->updates;
  }
  // Dispose may have waited for an in-progress transfer to release its lock.
  // It does not wait for queued Resource tasks to run. Already committed
  // updates keep their success; only uncommitted queue entries are failed here.
  for (const auto& update : updates) update->Cancel();
}

void Composition::EnqueueUpdate(
    std::string resource_id, PreloadedResource* resource, ResourceType type,
    std::unique_ptr<NapiOnResourceUpdateCallback> callback) {
  auto env = NapiEnv();
  auto data = std::static_pointer_cast<Data>(storage_);
  // Different page Envs can share a JS context. Inspect ownership before
  // ObtainStrongRef: foreign JS references must never enter this Env's storage.
  // NapiEnv() only reads the wrapper's stored Env pointer; it does not call
  // NAPI.
  const bool same_owner =
      resource->NapiEnv() == env && resource->GetHost() == GetHost();
  auto update = std::make_shared<CompositionOperation>(
      data, same_owner ? resource->storage_ : nullptr, type,
      std::move(resource_id), std::move(callback), env);
  EnqueueOperation(std::move(update), same_owner ? resource : nullptr);
}

void Composition::EnqueueOperation(std::shared_ptr<CompositionOperation> update,
                                   PreloadedResource* resource) {
  auto env = NapiEnv();
  auto data = std::static_pointer_cast<Data>(storage_);
  auto* refs = env.GetInstanceData<UpdateReferences>(kUpdateReferences);
  if (!refs) {
    refs = new UpdateReferences();
    env.SetInstanceData(kUpdateReferences, refs);
  }
  refs->entries.emplace(
      reinterpret_cast<uintptr_t>(update.get()),
      UpdateReferences::Entry{
          this, resource, ObtainStrongRef(),
          resource ? resource->ObtainStrongRef() : binding::ObjectRef{}});
  bool start = false;
  {
    std::lock_guard<std::mutex> lock(data->mutex);
    if (data->state != PreloadedResourceState::kAvailable) {
      // An already invalid handle must still settle this call. Do not append
      // behind a queue that disposal has stopped.
      start = true;
    } else {
      start = data->updates.empty();
      data->updates.push_back(update);
    }
  }
  if (start) update->Start();
}

void Composition::UpdateImage(
    std::string resource_id, PreloadedImage* image,
    std::unique_ptr<NapiOnResourceUpdateCallback> callback) {
  EnqueueUpdate(std::move(resource_id), image, ResourceType::kImage,
                std::move(callback));
}

void Composition::UpdateAudio(
    std::string resource_id, PreloadedAudio* audio,
    std::unique_ptr<NapiOnResourceUpdateCallback> callback) {
  EnqueueUpdate(std::move(resource_id), audio, ResourceType::kAudio,
                std::move(callback));
}

void Composition::UpdateVideo(
    std::string resource_id, PreloadedAlphaVideo* video,
    std::unique_ptr<NapiOnResourceUpdateCallback> callback) {
  EnqueueUpdate(std::move(resource_id), video, ResourceType::kVideo,
                std::move(callback));
}

void Composition::UpdateText(
    std::string layer_name, std::string text,
    std::unique_ptr<NapiOnResourceUpdateCallback> callback) {
  EnqueueOperation(std::make_shared<CompositionOperation>(
      std::static_pointer_cast<Data>(storage_), std::move(layer_name),
      std::move(text), std::move(callback), NapiEnv()));
}

void Composition::UpdateFont(
    std::string font_name, PreloadedFont* font,
    std::unique_ptr<NapiOnResourceUpdateCallback> callback) {
  EnqueueUpdate(std::move(font_name), font, ResourceType::kFont,
                std::move(callback));
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
