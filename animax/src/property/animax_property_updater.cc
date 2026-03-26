// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/animax_property_updater.h"

#include "src/layer/composition_layer.h"
#include "src/player/animax_renderer.h"
#include "src/property/resource_property_asset_updater.h"

namespace lynx {
namespace animax {

namespace {

ResourceType GetResourceType(ResourcePropertyType resource_type) {
  auto type_index = static_cast<uint16_t>(resource_type);
  if (type_index >= kImageRangeStart && type_index < kFontRangeStart) {
    return ResourceType::kImage;
  } else if (type_index >= kFontRangeStart && type_index < kVideoRangeStart) {
    return ResourceType::kFont;
  } else if (type_index >= kVideoRangeStart && type_index < kAudioRangeStart) {
    return ResourceType::kVideo;
  } else if (type_index >= kAudioRangeStart &&
             type_index <
                 static_cast<uint16_t>(ResourcePropertyType::kEnumEnd)) {
    return ResourceType::kAudio;
  } else {
    return ResourceType::kUnknown;
  }
}
}  // namespace

AnimaXPropertyUpdater::AnimaXPropertyUpdater(AnimaXRenderer& renderer)
    : renderer_(renderer) {}

AnimaXPropertyUpdater::~AnimaXPropertyUpdater() = default;

void AnimaXPropertyUpdater::GetKeysForKeyPath(
    std::unique_ptr<AnimaXKeyPath> key_path, AnimaXKeyPathCallback callback) {
  DCHECK(callback);
  if (!key_path) {
    callback({});
    return;
  }

  auto* layer = renderer_.GetCompositionLayer();
  if (!layer || !renderer_.GetComposition()) {
    callback({});
    return;
  }

  // Find elements matching the key path
  ResolvedKeyPathElements match_elements;
  layer->ResolveKeyPath(*key_path, 0, match_elements, AnimaXKeyPath());

  std::vector<AnimaXKeyPath> keys;
  for (const auto& resolved_element : match_elements) {
    keys.push_back(resolved_element.first);
  }

  callback(keys);
}

void AnimaXPropertyUpdater::UpdateLayerProperty(
    std::unique_ptr<LayerStaticRequest> request) {
  PropertyUpdateContext context{std::move(request)};
  // Check composition model and layer
  auto* layer = renderer_.GetCompositionLayer();
  if (!layer || !renderer_.GetComposition()) {
    context.InvokeErrorCallback(PropertyUpdateResult::kEnvInvalid);
    return;
  }

  if (context.IsValueNull()) {
    context.InvokeErrorCallback(PropertyUpdateResult::kValueInvalid);
    return;
  }

  ResolvedKeyPathElements match_elements;
  layer->ResolveKeyPath(context.GetKeyPath(), 0, match_elements,
                        AnimaXKeyPath());
  for (const auto& resolved_element : match_elements) {
    auto result = resolved_element.second->UpdateLayerProperty(context);
    if (result != PropertyUpdateResult::kSuccess) {
      context.AddErrorType(resolved_element.first.GetDescription(), result);
    }
  }

  context.InvokeCallback();
}

void AnimaXPropertyUpdater::SetResourceProperty(
    std::unique_ptr<ResourceUpdateRequest> request) {
  PropertyUpdateContext context{std::move(request)};

  // Check composition model and layer
  auto model = renderer_.GetComposition();
  if (!model || !renderer_.GetCompositionLayer()) {
    context.InvokeErrorCallback(PropertyUpdateResult::kEnvInvalid);
    return;
  }

  // Check updated resource type
  auto resource_type = GetResourceType(context.GetResourceType());
  if (resource_type == ResourceType::kUnknown) {
    context.InvokeErrorCallback(PropertyUpdateResult::kResourceTypeInvalid);
    return;
  }

  if (context.IsValueNull()) {
    context.InvokeErrorCallback(PropertyUpdateResult::kValueInvalid);
    return;
  }

  auto* resource_owner =
      model->GetResource(resource_type, context.GetResourceId());
  if (!resource_owner) {
    context.InvokeErrorCallback(PropertyUpdateResult::kResourceNotFound);
    return;
  }

  auto updater = ResourcePropertyAssetUpdator{context};
  resource_owner->AcceptVisitor(updater);

  context.InvokeCallback();
}

void AnimaXPropertyUpdater::AddLayerPropertyCallback(
    std::unique_ptr<LayerCallbackRequest> request) {
  PropertyUpdateContext context{std::move(request)};

  // Value callbacks only supported for layer properties
  if (!context.IsLayerRequest()) {
    context.InvokeErrorCallback(PropertyUpdateResult::kInvalidOperation);
    return;
  }

  // Check composition model and layer
  auto* layer = renderer_.GetCompositionLayer();
  if (!layer || !renderer_.GetComposition()) {
    context.InvokeErrorCallback(PropertyUpdateResult::kEnvInvalid);
    return;
  }

  // Find elements matching the key path
  ResolvedKeyPathElements match_elements;
  layer->ResolveKeyPath(context.GetKeyPath(), 0, match_elements,
                        AnimaXKeyPath());

  if (match_elements.empty()) {
    context.InvokeErrorCallback(PropertyUpdateResult::kKeyPathMismatched);
    return;
  }

  for (const auto& resolved_element : match_elements) {
    auto result = resolved_element.second->AddLayerPropertyCallback(context);
    if (result != PropertyUpdateResult::kSuccess) {
      context.AddErrorType(resolved_element.first.GetDescription(), result);
    }
  }

  context.InvokeCallback();
}

}  // namespace animax
}  // namespace lynx
