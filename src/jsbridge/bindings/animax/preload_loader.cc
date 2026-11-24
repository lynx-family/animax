// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/jsbridge/bindings/animax/preload_loader.h"

#include <atomic>
#include <cmath>
#include <limits>
#include <utility>

#include "src/base/thread/task_runner.h"
#include "src/jsbridge/bindings/animax/napi_alpha_video.h"
#include "src/jsbridge/bindings/animax/napi_composition.h"
#include "src/jsbridge/bindings/animax/napi_font.h"
#include "src/jsbridge/bindings/animax/napi_image.h"
#include "src/jsbridge/bindings/animax/napi_preloaded_audio.h"
#include "src/jsbridge/bindings/animax/napi_task_runner.h"
#include "src/jsbridge/bindings/animax/preload_host.h"
#include "src/jsbridge/bindings/animax/preloaded_resource.h"
#include "src/parser/alpha_player_asset/alpha_player_asset_parser.h"
#include "src/resource/composition_loader/composition_model_pipeline.h"
#include "src/resource/composition_loader/preloaded_asset_loader.h"
#include "src/resource/uri/uri_util.h"
#include "third_party/binding/napi/callback_helper.h"

#ifdef USE_PRIMJS_NAPI
#include "third_party/napi/include/primjs_napi_defines.h"
#endif

namespace lynx {
namespace animax {
namespace {

constexpr uint64_t kPreloadCallbacks = 0x8d9c625b0b568c76;

// The existing binding storage owns all JS references on the Env's thread and
// clears them during Env teardown. Background work holds native data only.
class LoadCallback : public std::enable_shared_from_this<LoadCallback> {
 public:
  LoadCallback(Napi::Function callback,
               const std::shared_ptr<PreloadHost>& host)
      : runner_(callback.Env()), host_(host) {
    auto env = callback.Env();
    auto* storage =
        env.GetInstanceData<binding::HolderStorage>(kPreloadCallbacks);
    if (!storage) {
      storage = new binding::HolderStorage();
      env.SetInstanceData(kPreloadCallbacks, storage);
    }
    storage->PushHolder(reinterpret_cast<uintptr_t>(this),
                        Napi::Persistent(callback));
    guard_ = storage->instance_guard();
  }

  // Native workers may finish on different threads. Queue at most one terminal
  // delivery to the originating Env; create_value only wraps the payload on its
  // JS thread after the callback holder and original host have been checked.
  // This controls result delivery, not cancellation of an in-flight load.
  template <typename CreateValue>
  void Complete(CreateValue create_value, std::string code = {},
                std::string message = {}) {
    if (completed_.exchange(true)) {
      return;
    }
    // Keep the callback identity alive until its queued JS completion runs, so
    // a later allocation cannot reuse the holder key while it is still stored.
    runner_.PostTask([self = shared_from_this(), guard = guard_, host = host_,
                      key = reinterpret_cast<uintptr_t>(this),
                      create_value = std::move(create_value),
                      code = std::move(code),
                      message = std::move(message)]() mutable {
      auto storage = guard.lock();
      if (!storage) {
        return;
      }
      auto callback = storage->Get()->PopHolder(key);
      if (callback.IsEmpty()) {
        return;
      }
      auto env = callback.Env();
      Napi::ContextScope context_scope(env);
      Napi::HandleScope handle_scope(env);
      auto owner = host.lock();
      // Page exit can invalidate the host before Env detach clears its slot.
      // Release the holder above and skip a page that has already exited. An
      // exit on another thread after this check can still receive the result,
      // but binding rechecks the host under its lock and rejects it.
      if (!owner || !owner->IsValid() || PreloadHost::FromEnv(env) != owner) {
        return;
      }
      Napi::Value value = env.Null();
      if (code.empty()) {
        value = create_value(env, owner);
      }
      Napi::Value ignored;
      binding::CallbackHelper::Invoke(
          callback, ignored, {},
          {Napi::Boolean::New(env, code.empty()), value,
           Napi::String::New(env, code), Napi::String::New(env, message)});
    });
  }

  void Fail(std::string code, std::string message) {
    Complete(
        [](Napi::Env env, const auto&) -> Napi::Value { return env.Null(); },
        std::move(code), std::move(message));
  }

 private:
  NapiTaskRunner runner_;
  std::weak_ptr<PreloadHost> host_;
  std::weak_ptr<binding::InstanceGuard> guard_;
  std::atomic<bool> completed_{false};
};

bool InvalidArgument(Napi::Env env, const char* message) {
  auto error = Napi::TypeError::New(env, message);
  error.Set("code", Napi::String::New(env, "InvalidArgument"));
  error.ThrowAsJavaScriptException();
  return false;
}

bool ReadArguments(const Napi::CallbackInfo& info, size_t uri_index,
                   std::string& uri, Napi::Object& options) {
  const size_t callback_index = uri_index + 2;
  if (info.Length() <= callback_index || !info[uri_index].IsString() ||
      !info[callback_index].IsFunction()) {
    return InvalidArgument(info.Env(),
                           "Expected a nonempty URI and a completion callback");
  }
  uri = info[uri_index].As<Napi::String>().Utf8Value();
  if (uri.find_first_not_of(" \t\r\n") == std::string::npos) {
    return InvalidArgument(info.Env(), "Expected a nonempty URI");
  }
  if (ParseUriScheme(uri) == UriInfo::Scheme::kUnknown) {
    return InvalidArgument(info.Env(), "Unsupported resource URI scheme");
  }
  options = Napi::Object::New(info.Env());
  const auto value = info[uri_index + 1];
  if (!value.IsUndefined()) {
    if (!value.IsObject() || value.IsNull() || value.IsArray()) {
      return InvalidArgument(info.Env(), "Expected an options object");
    }
    options = value.As<Napi::Object>();
  }
  return true;
}

bool ReadCompositionOptions(Napi::Object options, CompositionLoadInfo& result) {
  auto env = options.Env();
  auto audio = options.Get("enableAudio");
  if (env.IsExceptionPending()) return false;
  auto folder = options.Get("imageFolder");
  if (env.IsExceptionPending()) return false;
  auto polyfill = options.Get("srcPolyfill");
  if (env.IsExceptionPending()) return false;
  if ((!audio.IsUndefined() && !audio.IsBoolean()) ||
      (!folder.IsUndefined() && !folder.IsString())) {
    return InvalidArgument(
        env, "enableAudio must be boolean and imageFolder must be string");
  }
  result.enable_audio = audio.IsBoolean() && audio.As<Napi::Boolean>().Value();
  if (folder.IsString()) {
    result.source.image_folder = folder.As<Napi::String>().Utf8Value();
  }
  if (!polyfill.IsUndefined()) {
    if (!polyfill.IsObject() || polyfill.IsNull() || polyfill.IsArray()) {
      return InvalidArgument(env,
                             "srcPolyfill must map image slots to string URLs");
    }
    auto map = polyfill.As<Napi::Object>();
    auto keys = map.GetPropertyNames();
    if (env.IsExceptionPending()) return false;
    for (uint32_t i = 0; i < keys.Length(); ++i) {
      auto key = keys.Get(i).ToString().Utf8Value();
      if (env.IsExceptionPending()) return false;
      auto value = map.Get(key.c_str());
      if (env.IsExceptionPending()) return false;
      if (!value.IsString()) {
        return InvalidArgument(env, "srcPolyfill values must be strings");
      }
      result.source.polyfill.emplace(std::move(key),
                                     value.As<Napi::String>().Utf8Value());
    }
  }
  return !env.IsExceptionPending();
}

bool ReadImageOptions(Napi::Object options, ResourceRequest& request) {
  auto env = options.Env();
  auto width = options.Get("width");
  if (env.IsExceptionPending()) return false;
  auto height = options.Get("height");
  if (env.IsExceptionPending()) return false;
  request.image_size_mode = ImageSizeMode::kOriginal;
  if (width.IsUndefined() && height.IsUndefined()) {
    return true;
  }
  auto valid_dimension = [](const Napi::Value& value) {
    if (!value.IsNumber()) return false;
    const double number = value.As<Napi::Number>().DoubleValue();
    return std::isfinite(number) && number >= 1 &&
           number <= std::numeric_limits<int32_t>::max() &&
           std::floor(number) == number;
  };
  if (!valid_dimension(width) || !valid_dimension(height)) {
    return InvalidArgument(
        options.Env(),
        "width and height must both fit the decoder's positive int32 range");
  }
  request.width = width.As<Napi::Number>().Uint32Value();
  request.height = height.As<Napi::Number>().Uint32Value();
  request.image_size_mode = ImageSizeMode::kExact;
  return true;
}

std::shared_ptr<LoadCallback> MakeCallback(
    const Napi::CallbackInfo& info, size_t index,
    const std::shared_ptr<PreloadHost>& host) {
  auto callback = info[index].As<Napi::Function>();
  if (!host || !host->IsValid()) {
    callback.Call(
        {Napi::Boolean::New(info.Env(), false), info.Env().Null(),
         Napi::String::New(info.Env(), "EnvironmentUnavailable"),
         Napi::String::New(info.Env(), "Preload host is unavailable")});
    return nullptr;
  }
  return std::make_shared<LoadCallback>(callback, host);
}

Napi::Value LoadComposition(const Napi::CallbackInfo& info) {
  std::string uri;
  Napi::Object options;
  CompositionLoadInfo load_info;
  if (!ReadArguments(info, 0, uri, options) ||
      !ReadCompositionOptions(options, load_info)) {
    return info.Env().Undefined();
  }
  load_info.uri = uri;
  auto host = PreloadHost::FromEnv(info.Env());
  auto callback = MakeCallback(info, 2, host);
  if (!callback) return info.Env().Undefined();
  // The provider may complete on a platform thread. Carry native snapshots
  // across this boundary, then create the request's pipeline on Resource.
  host->GetCapabilities([callback, weak_host = std::weak_ptr<PreloadHost>(host),
                         uri = std::move(uri),
                         load_info = std::move(load_info)](
                            PreloadHostCapabilities capabilities,
                            std::string error) mutable {
    if (!error.empty()) {
      callback->Fail("EnvironmentUnavailable", std::move(error));
      return;
    }
    GetAnimaXResourceThread()->PostTask([callback, weak_host,
                                         capabilities = std::move(capabilities),
                                         uri = std::move(uri),
                                         load_info =
                                             std::move(load_info)]() mutable {
      auto host = weak_host.lock();
      if (!host || !host->IsValid()) {
        callback->Fail("EnvironmentUnavailable", "Preload host is unavailable");
        return;
      }
      // Reuse the page's loaders, but keep pipeline options and source metadata
      // local to this request. Parsing uses the captured host scale; no View or
      // platform screen API is consulted from this thread.
      auto pipeline = CompositionModelPipeline::MakeForPreload(
          std::move(capabilities.resource_loader),
          std::move(capabilities.unzip_loader), load_info.enable_audio);
      load_info.scale = capabilities.scale;
      pipeline->LoadCompositionModelOnlyFromURI(
          std::move(uri), load_info.scale,
          [callback, pipeline, load_info = std::move(load_info)](
              CompositionModelResponse result, LoaderError error) mutable {
            if (error || (!result.model && !result.alpha_player_config)) {
              // Match loadResource: only a missing capability is unsupported.
              const char* code =
                  error.code == kNoSuchLoader ? "UnsupportedCapability"
                  : error.code == kCompositionParserError ? "ParseFailed"
                                                          : "LoadFailed";
              callback->Fail(code, error.message.empty()
                                       ? "Composition loading failed"
                                       : std::move(error.message));
              return;
            }
            load_info.source.main_uri = std::move(result.main_uri);
            load_info.source.base_uri = std::move(result.base_uri);
            load_info.model_type = result.model_type;
            // AlphaPlayer packages retain only validated configuration here.
            // Accounting starts when prepared payloads are added.
            callback->Complete(
                [model = std::move(result.model),
                 config = std::move(result.alpha_player_config),
                 load_info = std::move(load_info)](
                    Napi::Env env, const auto& host) mutable -> Napi::Value {
                  std::unique_ptr<Composition> resource;
                  if (model) {
                    resource = std::make_unique<Composition>(
                        std::move(model), std::move(load_info));
                  } else {
                    resource = std::make_unique<Composition>(
                        std::move(config), std::move(load_info));
                  }
                  resource->SetHost(host);
                  return NapiComposition::Wrap(std::move(resource), env);
                });
          });
    });
  });
  return info.Env().Undefined();
}

Napi::Value LoadResource(const Napi::CallbackInfo& info) {
  ResourceType type = ResourceType::kUnknown;
  if (info[0].IsString()) {
    auto name = info[0].As<Napi::String>().Utf8Value();
    if (name == "image")
      type = ResourceType::kImage;
    else if (name == "audio")
      type = ResourceType::kAudio;
    else if (name == "alphaVideo")
      type = ResourceType::kVideo;
    else if (name == "font")
      type = ResourceType::kFont;
  }
  if (type == ResourceType::kUnknown) {
    InvalidArgument(info.Env(),
                    "type must be image, audio, alphaVideo or font");
    return info.Env().Undefined();
  }
  const bool image = type == ResourceType::kImage;
  std::string uri;
  Napi::Object options;
  ResourceRequest request{};
  if (!ReadArguments(info, 1, uri, options) ||
      (image && !ReadImageOptions(options, request))) {
    return info.Env().Undefined();
  }
  if (!image) {
    auto keys = options.GetPropertyNames();
    if (info.Env().IsExceptionPending()) return info.Env().Undefined();
    if (keys.Length() != 0) {
      InvalidArgument(info.Env(),
                      "Only image loading accepts resource options");
      return info.Env().Undefined();
    }
  }
  request.type = image ? ResourceRequestType::kLoadBitmap
                 : type == ResourceType::kFont
                     ? ResourceRequestType::kLoadRawData
                     : ResourceRequestType::kDownloadToLocal;
  request.uri_info = {
      ParseUriScheme(uri),
      image                          ? UriInfo::ContentType::kImage
      : type == ResourceType::kAudio ? UriInfo::ContentType::kAudio
      : type == ResourceType::kFont  ? UriInfo::ContentType::kFont
                                     : UriInfo::ContentType::kVideo,
      std::move(uri)};
  auto host = PreloadHost::FromEnv(info.Env());
  auto callback = MakeCallback(info, 3, host);
  if (!callback) return info.Env().Undefined();
  // Independent resources need the page's resource service, not a Composition
  // or Player. Start loading on Resource, as model requests do, and recheck the
  // host after dispatch. Only LoadCallback publishes a wrapper to the JS Env.
  host->GetCapabilities([callback, weak_host = std::weak_ptr<PreloadHost>(host),
                         type, request = std::move(request)](
                            PreloadHostCapabilities capabilities,
                            std::string error) mutable {
    if (!error.empty()) {
      callback->Fail("EnvironmentUnavailable", std::move(error));
      return;
    }
    GetAnimaXResourceThread()->PostTask(
        [callback, weak_host, type, request = std::move(request),
         loader = std::move(capabilities.resource_loader)]() mutable {
          auto host = weak_host.lock();
          if (!host || !host->IsValid()) {
            callback->Fail("EnvironmentUnavailable",
                           "Preload host is unavailable");
            return;
          }
          auto font_uri = type == ResourceType::kFont ? request.uri_info.uri
                                                      : std::string{};
          LoadPreloadedAsset(
              type, std::move(request), std::move(loader),
              [callback, type, font_uri = std::move(font_uri)](
                  PreloadedAssetResponse response) mutable {
                if (response.result.error) {
                  callback->Fail(
                      response.result.error.code == kNoSuchLoader
                          ? "UnsupportedCapability"
                      : response.stage == PreloadedAssetStage::kPrepare
                          ? "PreparationFailed"
                          : "LoadFailed",
                      std::move(response.result.error.message));
                  return;
                }
                callback->Complete([response = std::move(response), type,
                                    font_uri = std::move(font_uri)](
                                       Napi::Env env, const auto& host) mutable
                                   -> Napi::Value {
                  if (type == ResourceType::kImage) {
                    auto resource = std::make_unique<PreloadedImage>(
                        std::move(response.bitmap));
                    resource->SetHost(host);
                    return NapiImage::Wrap(std::move(resource), env);
                  }
                  if (type == ResourceType::kAudio) {
                    auto resource = std::make_unique<PreloadedAudio>(
                        std::move(response.audio), response.payload_bytes);
                    resource->SetHost(host);
                    return NapiPreloadedAudio::Wrap(std::move(resource), env);
                  }
                  if (type == ResourceType::kFont) {
                    auto resource = std::make_unique<PreloadedFont>(
                        std::move(response.font_data), std::move(font_uri),
                        response.payload_bytes);
                    resource->SetHost(host);
                    return NapiFont::Wrap(std::move(resource), env);
                  }
                  auto resource = std::make_unique<PreloadedAlphaVideo>(
                      std::move(response.video), response.payload_bytes);
                  resource->SetHost(host);
                  return NapiAlphaVideo::Wrap(std::move(resource), env);
                });
              });
        });
  });
  return info.Env().Undefined();
}

}  // namespace

void InstallPreloadLoaders(Napi::Env env, Napi::Object& module) {
  module.Set("loadComposition", Napi::Function::New(env, LoadComposition));
  module.Set("loadResource", Napi::Function::New(env, LoadResource));
}

}  // namespace animax
}  // namespace lynx
