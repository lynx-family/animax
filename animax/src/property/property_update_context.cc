// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/property_update_context.h"

#include "include/property/animax_key_path.h"
#include "include/property/animax_value_param.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

PropertyUpdateContext::PropertyUpdateContext(
    std::unique_ptr<PropertyUpdateRequest> request)
    : request_(std::move(request)) {}

void PropertyUpdateContext::InvokeErrorCallback(
    PropertyUpdateResult error_type) {
  SetErrorType(error_type);
  InvokeCallback();
}

void PropertyUpdateContext::InvokeCallback() {
  DCHECK(request_);
  request_->InvokeCallback(response_);
}

bool PropertyUpdateContext::IsValueNull() const {
  if (IsStaticLayerRequest()) {
    auto* request = GetRequest<const LayerStaticRequest>();
    return request == nullptr || request->GetValue().IsNull();
  } else if (IsResourceRequest()) {
    auto* request = GetRequest<const ResourceUpdateRequest>();
    return request == nullptr || request->GetValue().IsNull();
  } else if (IsLayerCallbackRequest()) {
    auto* request = GetRequest<const LayerCallbackRequest>();
    return request == nullptr || request->GetValueCallback() == nullptr;
  } else {
    return true;
  }
}

void PropertyUpdateContext::SetErrorType(PropertyUpdateResult error_type) {
  response_.SetErrorType(error_type);
}

void PropertyUpdateContext::AddErrorType(std::string target,
                                         PropertyUpdateResult error_type) {
  response_.AddErrorType(std::move(target), error_type);
}

PropertyUpdateResponse& PropertyUpdateContext::GetResponse() {
  return response_;
}

bool PropertyUpdateContext::HasValueCallback() const {
  if (!IsLayerCallbackRequest()) {
    return false;
  }
  auto* request = GetRequest<const LayerCallbackRequest>();
  return request != nullptr && request->GetValueCallback() != nullptr;
}

std::shared_ptr<AnimaXValueCallback> PropertyUpdateContext::GetValueCallback() {
  DCHECK(IsLayerCallbackRequest());
  auto* request = GetRequest<LayerCallbackRequest>();
  return request ? request->GetValueCallback() : nullptr;
}

LayerPropertyType PropertyUpdateContext::GetLayerType() const {
  DCHECK(IsLayerRequest());
  if (IsStaticLayerRequest()) {
    return GetRequest<const LayerStaticRequest>()->GetLayerType();
  } else {
    return GetRequest<const LayerCallbackRequest>()->GetLayerType();
  }
}

ResourcePropertyType PropertyUpdateContext::GetResourceType() const {
  DCHECK(IsResourceRequest());
  return GetRequest<const ResourceUpdateRequest>()->GetResourceType();
}

const AnimaXKeyPath& PropertyUpdateContext::GetKeyPath() const {
  DCHECK(IsLayerRequest());
  if (IsStaticLayerRequest()) {
    return GetRequest<const LayerStaticRequest>()->GetKeyPath();
  } else {
    return GetRequest<const LayerCallbackRequest>()->GetKeyPath();
  }
}

const std::string& PropertyUpdateContext::GetResourceId() const {
  DCHECK(IsResourceRequest());
  return GetRequest<const ResourceUpdateRequest>()->GetResourceId();
}

const AnimaXValueParam& PropertyUpdateContext::GetValue() const {
  DCHECK(IsStaticLayerRequest() || IsResourceRequest());
  if (IsStaticLayerRequest()) {
    return GetRequest<const LayerStaticRequest>()->GetValue();
  } else {
    return GetRequest<const ResourceUpdateRequest>()->GetValue();
  }
}

bool PropertyUpdateContext::IsLayerRequest() const {
  return IsStaticLayerRequest() || IsLayerCallbackRequest();
}

bool PropertyUpdateContext::IsStaticLayerRequest() const {
  DCHECK(request_);
  return request_->GetRequestType() ==
         PropertyUpdateRequest::Type::kLayerStatic;
}

bool PropertyUpdateContext::IsResourceRequest() const {
  DCHECK(request_);
  return request_->GetRequestType() == PropertyUpdateRequest::Type::kResource;
}

bool PropertyUpdateContext::IsLayerCallbackRequest() const {
  DCHECK(request_);
  return request_->GetRequestType() ==
         PropertyUpdateRequest::Type::kLayerCallback;
}

}  // namespace animax
}  // namespace lynx
