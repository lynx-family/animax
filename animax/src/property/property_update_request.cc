// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/property/property_update_request.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

// Base PropertyUpdateRequest implementation
PropertyUpdateRequest::PropertyUpdateRequest(Type type,
                                             AnimaXPropertyCallback callback)
    : callback_(std::move(callback)), type_(type) {}

void PropertyUpdateRequest::InvokeCallback(
    const PropertyUpdateResponse& response) const {
  if (callback_) {
    callback_(response);
  }
}

PropertyUpdateRequest::Type PropertyUpdateRequest::GetRequestType() const {
  return type_;
}

// LayerStaticRequest implementation
std::unique_ptr<LayerStaticRequest> LayerStaticRequest::Make(
    LayerPropertyType type, std::unique_ptr<AnimaXKeyPath> key_path,
    std::unique_ptr<AnimaXValueParam> value, AnimaXPropertyCallback callback) {
  return std::unique_ptr<LayerStaticRequest>(new LayerStaticRequest(
      type, std::move(key_path), std::move(value), std::move(callback)));
}

LayerStaticRequest::LayerStaticRequest(LayerPropertyType type,
                                       std::unique_ptr<AnimaXKeyPath> key_path,
                                       std::unique_ptr<AnimaXValueParam> value,
                                       AnimaXPropertyCallback callback)
    : PropertyUpdateRequest(Type::kLayerStatic, std::move(callback)),
      layer_type_(type),
      key_path_(std::move(key_path)),
      value_(std::move(value)) {}

LayerPropertyType LayerStaticRequest::GetLayerType() const {
  return layer_type_;
}

const AnimaXKeyPath& LayerStaticRequest::GetKeyPath() const {
  DCHECK(key_path_);
  return *key_path_;
}

const AnimaXValueParam& LayerStaticRequest::GetValue() const {
  DCHECK(value_);
  return *value_;
}

// LayerCallbackRequest implementation
std::unique_ptr<LayerCallbackRequest> LayerCallbackRequest::Make(
    LayerPropertyType type, std::unique_ptr<AnimaXKeyPath> key_path,
    std::shared_ptr<AnimaXValueCallback> value_callback,
    AnimaXPropertyCallback callback) {
  return std::unique_ptr<LayerCallbackRequest>(
      new LayerCallbackRequest(type, std::move(key_path),
                               std::move(value_callback), std::move(callback)));
}

LayerCallbackRequest::LayerCallbackRequest(
    LayerPropertyType type, std::unique_ptr<AnimaXKeyPath> key_path,
    std::shared_ptr<AnimaXValueCallback> value_callback,
    AnimaXPropertyCallback callback)
    : PropertyUpdateRequest(Type::kLayerCallback, std::move(callback)),
      layer_type_(type),
      key_path_(std::move(key_path)),
      value_callback_(std::move(value_callback)) {}

LayerPropertyType LayerCallbackRequest::GetLayerType() const {
  return layer_type_;
}

const AnimaXKeyPath& LayerCallbackRequest::GetKeyPath() const {
  DCHECK(key_path_);
  return *key_path_;
}

std::shared_ptr<AnimaXValueCallback> LayerCallbackRequest::GetValueCallback()
    const {
  return value_callback_;
}

// ResourceUpdateRequest implementation
std::unique_ptr<ResourceUpdateRequest> ResourceUpdateRequest::Make(
    ResourcePropertyType type, std::string resource_id,
    std::unique_ptr<AnimaXValueParam> value, AnimaXPropertyCallback callback) {
  return std::unique_ptr<ResourceUpdateRequest>(new ResourceUpdateRequest(
      type, std::move(resource_id), std::move(value), std::move(callback)));
}

ResourceUpdateRequest::ResourceUpdateRequest(
    ResourcePropertyType type, std::string resource_id,
    std::unique_ptr<AnimaXValueParam> value, AnimaXPropertyCallback callback)
    : PropertyUpdateRequest(Type::kResource, std::move(callback)),
      resource_type_(type),
      resource_id_(std::move(resource_id)),
      value_(std::move(value)) {}

ResourcePropertyType ResourceUpdateRequest::GetResourceType() const {
  return resource_type_;
}

const std::string& ResourceUpdateRequest::GetResourceId() const {
  return resource_id_;
}

const AnimaXValueParam& ResourceUpdateRequest::GetValue() const {
  DCHECK(value_);
  return *value_;
}

}  // namespace animax
}  // namespace lynx
