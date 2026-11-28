// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOADED_RESOURCE_H_
#define ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOADED_RESOURCE_H_

#include <cstdint>
#include <memory>
#include <string>

#include "include/resource/uri_info.h"
#include "src/resource/asset/asset.h"
#include "src/resource/composition_loader/composition_task.h"
#include "third_party/binding/common/base.h"
#include "third_party/binding/napi/shim/shim_napi.h"

namespace lynx {
namespace animax {

class AudioAsset;
class Bitmap;
class CompositionModel;
class VideoAsset;
class PreloadHost;
class NapiOnResourceUpdateCallback;
class CompositionOperation;
class PreloadedImage;
class PreloadedAudio;
class PreloadedAlphaVideo;
class PreloadedFont;
class RawData;

// Request snapshots needed to resolve default assets after independent loading.
struct CompositionLoadInfo {
  std::string uri;
  MainResourceUriInfo source;
  CompositionModelType model_type = CompositionModelType::kInvalid;
  float scale = 0;
  bool enable_audio = false;
};

// Consumption requires transferring payload ownership; it is not disposal.
enum class PreloadedResourceState { kAvailable, kConsumed, kDisposed };

// JS references and external-memory notifications stay on the originating Env.
// Resource tasks retain native storage independently: Env teardown can destroy
// the JS wrapper while a queued task is still completing native cleanup.
class PreloadedResource : public binding::ImplBase {
 public:
  PreloadedResource(const PreloadedResource&) = delete;
  PreloadedResource& operator=(const PreloadedResource&) = delete;

  PreloadedResourceState GetState() const;
  int64_t GetMemoryUsage() const;
  std::shared_ptr<PreloadHost> GetHost() const;
  void SetHost(const std::shared_ptr<PreloadHost>& host);

  void OnWrapped() override;
  void Dispose() override;

 protected:
  struct Storage;
  explicit PreloadedResource(std::shared_ptr<Storage> storage);
  void SyncExternalMemory();
  std::shared_ptr<Storage> storage_;

 private:
  friend class CompositionOperation;
  friend class Composition;
};

// Owns a parsed model or an AlphaPlayer configuration, without a Player or
// Renderer. AlphaPlayer timing is unavailable until a prepared video is added.
// Memory accounting covers prepared payloads only, not the model structure.
class Composition : public PreloadedResource {
 public:
  explicit Composition(std::shared_ptr<CompositionModel> model,
                       CompositionLoadInfo load_info = {});
  explicit Composition(std::shared_ptr<const AlphaPlayerConfig> config,
                       CompositionLoadInfo load_info = {});
  ~Composition() override;
  // Borrowed inspection is valid only while no native operation is running.
  // A valid configuration-only Composition has no model yet. Only a completed
  // model can later be installed in a Renderer.
  CompositionModel* GetModel() const;
  const AlphaPlayerConfig* GetAlphaPlayerConfig() const;
  const CompositionLoadInfo& GetLoadInfo() const;

  void UpdateImage(std::string resource_id, PreloadedImage* image,
                   std::unique_ptr<NapiOnResourceUpdateCallback> callback);
  void UpdateAudio(std::string resource_id, PreloadedAudio* audio,
                   std::unique_ptr<NapiOnResourceUpdateCallback> callback);
  void UpdateVideo(std::string resource_id, PreloadedAlphaVideo* video,
                   std::unique_ptr<NapiOnResourceUpdateCallback> callback);
  // Edits all text keyframes in matching used layers before binding. Shares
  // the resource FIFO; a failed edit leaves every matching layer unchanged.
  void UpdateText(std::string layer_name, std::string text,
                  std::unique_ptr<NapiOnResourceUpdateCallback> callback);
  // Transfers prepared font bytes while preserving template names and style.
  // A failed transfer leaves the candidate and existing font unchanged.
  void UpdateFont(std::string font_name, PreloadedFont* font,
                  std::unique_ptr<NapiOnResourceUpdateCallback> callback);
  void Dispose() override;

 private:
  struct Data;
  friend class CompositionOperation;
  void EnqueueOperation(std::shared_ptr<CompositionOperation> operation,
                        PreloadedResource* resource = nullptr);
  void EnqueueUpdate(std::string resource_id, PreloadedResource* resource,
                     ResourceType type,
                     std::unique_ptr<NapiOnResourceUpdateCallback> callback);
};

class PreloadedImage : public PreloadedResource {
 public:
  explicit PreloadedImage(std::unique_ptr<Bitmap> bitmap);
  ~PreloadedImage() override;
  const Bitmap* GetBitmap() const;

 private:
  struct Data;
  friend class CompositionOperation;
};

class PreloadedAudio : public PreloadedResource {
 public:
  PreloadedAudio(std::shared_ptr<AudioAsset> asset, int64_t payload_bytes);
  ~PreloadedAudio() override;
  AudioAsset* GetAsset() const;

 private:
  struct Data;
  friend class CompositionOperation;
};

// Internal payload prepared without a template. Rendering creates its Font
// lazily after transfer; the template still supplies family/name/style.
class PreloadedFont : public PreloadedResource {
 public:
  PreloadedFont(std::unique_ptr<RawData> data, std::string uri,
                int64_t payload_bytes);
  ~PreloadedFont() override;
  const RawData* GetData() const;

 private:
  struct Data;
  friend class CompositionOperation;
};

// The public AlphaVideo wrapper owns prepared MP4 data. RGB/alpha regions
// come from the target composition when the resource is injected.
class PreloadedAlphaVideo : public PreloadedResource {
 public:
  PreloadedAlphaVideo(std::shared_ptr<VideoAsset> asset, int64_t payload_bytes);
  ~PreloadedAlphaVideo() override;
  VideoAsset* GetAsset() const;

 private:
  struct Data;
  friend class CompositionOperation;
};

void InstallPreloadBindings(Napi::Env env, Napi::Object& module);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOADED_RESOURCE_H_
