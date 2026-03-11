// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/web/resource_loader_web.h"

#include "src/base/log/log.h"
#include "src/resource/loader/exec_loader.h"
#include "src/resource/loader/lambda_loader.h"

namespace lynx {
namespace animax {
base::NoDestructor<ResourceLoaderWeb::CallbackManager>
    ResourceLoaderWeb::callback_manager;

ResourceLoaderWeb::ResourceLoaderWeb()
    : loader_id_(callback_manager->next_loader_id++) {}
ResourceLoaderWeb::~ResourceLoaderWeb() = default;

void ResourceLoaderWeb::SetLoaderImpl(ResourceLoaderImpl impl) {
  loader_impl_ = impl;
}

void ResourceLoaderWeb::Load(ResourceRequest request, CallbackType callback) {
  if (!loader_impl_) {
    callback(ResourceResponse{},
             LoaderError{.code = kInvalidLoaderRequest,
                         .message = "Load fail, loader impl is null."});
    return;
  }

  switch (request.type) {
    case ResourceRequestType::kLoadRawData:
    case ResourceRequestType::kLoadBitmap: {
      const int32_t callback_id = callback_manager->next_id++;
      auto callback_info = std::make_shared<CallbackInfo>(
          CallbackInfo{.type = request.type, .callback = std::move(callback)});
      callback_list_.push_back(callback_info);
      callback_manager->callbacks[callback_id] = callback_info;
      // ANIMAX_LOGI("Load resource, callback_id: " << callback_id);
      loader_impl_(request.uri_info.uri, static_cast<int32_t>(request.type),
                   callback_id);
    } break;
    case ResourceRequestType::kDownloadToLocal:
      callback(
          ResourceResponse{},
          LoaderError{
              .code = kInvalidLoaderRequest,
              .message = "Load fail, download to local is not supported."});
      break;
    default:
      callback(ResourceResponse{},
               LoaderError{.code = kInvalidLoaderRequest,
                           .message = "Load fail, invalid request type."});
      break;
  }
}

void ResourceLoaderWeb::OnResourceLoaded(const ResourceLoadResult& result) {
  auto& callbacks = ResourceLoaderWeb::callback_manager->callbacks;
  auto it = callbacks.find(result.callback_id);
  if (it == callbacks.end()) {
    ANIMAX_LOGW("OnResourceLoaded, invalid callback id "
                << result.callback_id
                << " callback_list size: " << callbacks.size());
    return;
  }
  auto info = it->second.lock();
  callbacks.erase(it);
  if (!info || info->callback == nullptr) {
    ANIMAX_LOGW("OnResourceLoaded, callback is expired, callback_id: "
                << result.callback_id);
    return;
  }

  if (!result.success) {
    ANIMAX_LOGE("OnResourceLoaded, load failed, callback_id: "
                << result.callback_id << " error: " << result.error_message);
    info->callback(ResourceResponse{},
                   LoaderError{.code = kInvalidLoaderRequest,
                               .message = "Load fail, invalid request type."});
    return;
  }

  LoaderErrorCode error_code =
      (info->type == ResourceRequestType::kLoadBitmap ? kInvalidImageData
                                                      : kInvalidRawData);
  if (result.size <= 0 || result.data == nullptr) {
    ANIMAX_LOGE("OnResourceLoaded, Uint8Array length is zero, callback_id: "
                << result.callback_id);
    info->callback(
        ResourceResponse{},
        LoaderError{.code = error_code, .message = "Empty data received."});
    return;
  }

  if (info->type == ResourceRequestType::kLoadRawData) {
    uint8_t* copy = new uint8_t[result.size];
    memcpy(copy, result.data, result.size);
    auto raw_data = RawData::MakeRawData(
        copy, result.size,
        [](const void* ptr) { delete[] static_cast<const uint8_t*>(ptr); },
        copy);
    info->callback(ResourceResponse{.payload = MakeRawDataResourcePayload(
                                        std::move(raw_data))},
                   LoaderError{});
  } else {
    DCHECK(info->type == ResourceRequestType::kLoadBitmap);
    uint8_t* copy = new uint8_t[result.size];
    memcpy(copy, result.data, result.size);
    auto bitmap = Bitmap::Make(
        result.width, result.height, copy,
        [](const void* ptr) { delete[] static_cast<const uint8_t*>(ptr); },
        copy, BitmapFormat::kRGBA, BitmapAlphaType::kUnpremul_AlphaType);
    info->callback(ResourceResponse{.payload = MakeBitmapResourcePayload(
                                        std::move(bitmap))},
                   LoaderError{});
  }
}

}  // namespace animax
}  // namespace lynx
