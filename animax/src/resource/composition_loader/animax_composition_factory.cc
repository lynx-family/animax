// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/animax_composition_factory.h"

namespace lynx {
namespace animax {
AnimaXCompositionFactory& AnimaXCompositionFactory::Instance() {
  static base::NoDestructor<AnimaXCompositionFactory> instance;
  return *instance;
}

void AnimaXCompositionFactory::CreateLoader(
    std::string scope,
    Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
    Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader) {
  auto loader_actor = AnimaXCompositionLoader::Create();
  loader_actor->Act([resource_loader = std::move(resource_loader),
                     unzip_loader = std::move(unzip_loader)](auto& loader) {
    loader->Init(std::move(resource_loader), unzip_loader);
  });
  composition_loaders_[scope] = loader_actor;
}

std::shared_ptr<shell::LynxActor<AnimaXCompositionLoader>>
AnimaXCompositionFactory::GetLoader(const std::string& scope) {
  auto it = composition_loaders_.find(scope);
  if (it != composition_loaders_.end()) {
    return it->second;
  }
  return nullptr;
}

void AnimaXCompositionFactory::LoadJson(std::string scope, std::string json_str,
                                        float scale,
                                        CompositionCallback callback) {
  if (!callback) {
    return;
  }

  auto loader_actor = GetLoader(scope);
  if (!loader_actor) {
    callback(nullptr, "Loader not initialized for scope: " + scope);
    return;
  }

  if (json_str.empty()) {
    callback(nullptr, "JSON string is empty");
    return;
  }

  loader_actor->Act([json_str = std::move(json_str), scale,
                     callback = std::move(callback)](auto& loader) mutable {
    loader->LoadCompositionModelFromJSONString(
        json_str, scale,
        [callback = std::move(callback)](CompositionAssetResponse res,
                                         LoaderError error) mutable {
          callback(res.model, error.message);
        });
  });
}

void AnimaXCompositionFactory::LoadUri(std::string scope, std::string uri,
                                       float scale,
                                       CompositionCallback callback) {
  if (!callback) {
    return;
  }

  auto loader_actor = GetLoader(scope);
  if (!loader_actor) {
    callback(nullptr, "Loader not initialized for scope: " + scope);
    return;
  }

  if (uri.empty()) {
    callback(nullptr, "URI string is empty");
    return;
  }

  loader_actor->Act([uri = std::move(uri), scale,
                     callback = std::move(callback)](auto& loader) mutable {
    loader->LoadCompositionModelFromURI(
        uri, scale,
        [callback = std::move(callback)](CompositionAssetResponse res,
                                         LoaderError error) mutable {
          callback(res.model, error.message);
        });
  });
}

void AnimaXCompositionFactory::Release() { composition_loaders_.clear(); }

}  // namespace animax
}  // namespace lynx
