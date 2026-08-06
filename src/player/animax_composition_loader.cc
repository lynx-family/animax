// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/animax_composition_loader.h"

#include "src/base/thread/task_runner.h"
#include "src/resource/composition_loader/composition_model_pipeline.h"

namespace lynx {
namespace animax {

std::shared_ptr<shell::LynxActor<AnimaXCompositionLoader>>
AnimaXCompositionLoader::Create() {
  auto loader =
      std::unique_ptr<AnimaXCompositionLoader>(new AnimaXCompositionLoader());
  return std::make_shared<shell::LynxActor<AnimaXCompositionLoader>>(
      std::move(loader), GetAnimaXResourceThread());
}

void AnimaXCompositionLoader::Init(ResourceLoader::Ptr resource_loader,
                                   UnzipLoader::Ptr unzip_loader) {
  pipeline_->Init(resource_loader, unzip_loader);
}

void AnimaXCompositionLoader::SetSrcPolyfill(
    std::unordered_map<std::string, std::string> polyfill) {
  pipeline_->SetSrcPolyfill(std::move(polyfill));
}

void AnimaXCompositionLoader::SetImageFolder(std::string image_folder) {
  pipeline_->SetImageFolder(std::move(image_folder));
}

void AnimaXCompositionLoader::SetHasDynamicResource(bool has_dynamic_resource) {
  pipeline_->SetHasDynamicResource(has_dynamic_resource);
}

void AnimaXCompositionLoader::SetEnableAudio(bool enable) {
  pipeline_->SetEnableAudio(enable);
}

void AnimaXCompositionLoader::SetAllowExtensionlessJson(bool allow) {
  pipeline_->SetAllowExtensionlessJson(allow);
}

void AnimaXCompositionLoader::LoadCompositionModelFromURI(
    std::string uri, float scale, AssetLoader::CallbackType callback) {
  pipeline_->LoadCompositionModelFromURI(std::move(uri), scale,
                                         std::move(callback));
}

void AnimaXCompositionLoader::LoadCompositionModelFromJSONString(
    std::string json_str, float scale, AssetLoader::CallbackType callback) {
  pipeline_->LoadCompositionModelFromJSONString(std::move(json_str), scale,
                                                std::move(callback));
}

void AnimaXCompositionLoader::LoadCompositionModelAsset(
    std::shared_ptr<CompositionModel> model,
    AssetLoader::CallbackType callback) {
  pipeline_->LoadCompositionModelAsset(std::move(model), std::move(callback));
}

void AnimaXCompositionLoader::Trace(TraceEventType type) {
  pipeline_->Trace(type);
}

TimestampArray AnimaXCompositionLoader::ExportTimestamps() const {
  return pipeline_->ExportTimestamps();
}

}  // namespace animax
}  // namespace lynx
