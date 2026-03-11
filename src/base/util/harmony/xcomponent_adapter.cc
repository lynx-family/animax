// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/harmony/xcomponent_adapter.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

namespace {
void OnSurfaceCreatedCB(OH_NativeXComponent *component, void *window) {
  XComponentAdapter::GetInstance().OnSurfaceCreated(component, window);
}

void OnSurfaceChangedCB(OH_NativeXComponent *component, void *window) {
  XComponentAdapter::GetInstance().OnSurfaceChanged(component, window);
}

void OnSurfaceDestroyedCB(OH_NativeXComponent *component, void *window) {
  XComponentAdapter::GetInstance().OnSurfaceDestroyed(component, window);
}

void DispatchTouchEventCB(OH_NativeXComponent *component, void *window) {
  // ignore
}
}  // namespace

XComponentAdapter::XComponentAdapter()
    : callback_({
          .OnSurfaceCreated = OnSurfaceCreatedCB,
          .OnSurfaceChanged = OnSurfaceChangedCB,
          .OnSurfaceDestroyed = OnSurfaceDestroyedCB,
          .DispatchTouchEvent = DispatchTouchEventCB,
      }) {}

XComponentAdapter &XComponentAdapter::GetInstance() {
  static XComponentAdapter instance;
  return instance;
}

void XComponentAdapter::RegisterCallback(OH_NativeXComponent *xcomponent,
                                         XComponentCallback *callback) {
  ANIMAX_LOGI("XComponentAdapter::RegisterCallback with component: "
              << xcomponent << " callback: " << callback);
  callback_map_[xcomponent] = callback;
  OH_NativeXComponent_RegisterCallback(xcomponent, &callback_);
}

void XComponentAdapter::UnregisterCallback(OH_NativeXComponent *xcomponent) {
  ANIMAX_LOGI(
      "XComponentAdapter::UnregisterCallback with component: " << xcomponent);
  OH_NativeXComponent_RegisterCallback(xcomponent, nullptr);
  callback_map_.erase(xcomponent);
}

void XComponentAdapter::OnSurfaceCreated(OH_NativeXComponent *component,
                                         void *window) {
  ANIMAX_LOGI("XComponentAdapter::OnSurfaceCreated with component: "
              << component << " window: " << window)
  auto iter = callback_map_.find(component);
  if (iter != callback_map_.end()) {
    iter->second->OnSurfaceCreated(component, window);
  } else {
    ANIMAX_LOGE("XComponentAdapter::OnSurfaceCreated: listener not found");
  }
}

void XComponentAdapter::OnSurfaceChanged(OH_NativeXComponent *component,
                                         void *window) {
  ANIMAX_LOGI("XComponentAdapter::OnSurfaceChanged with component: "
              << component << " window: " << window)
  auto iter = callback_map_.find(component);
  if (iter != callback_map_.end()) {
    iter->second->OnSurfaceChanged(component, window);
  } else {
    ANIMAX_LOGE("XComponentAdapter::OnSurfaceChanged: listener not found");
  }
}

void XComponentAdapter::OnSurfaceDestroyed(OH_NativeXComponent *component,
                                           void *window) {
  ANIMAX_LOGI("XComponentAdapter::OnSurfaceDestroyed with component: "
              << component << " window: " << window)
  auto iter = callback_map_.find(component);
  if (iter != callback_map_.end()) {
    iter->second->OnSurfaceDestroyed(component, window);
  } else {
    ANIMAX_LOGE("XComponentAdapter::OnSurfaceDestroyed: listener not found");
  }
}

}  // namespace animax
}  // namespace lynx
