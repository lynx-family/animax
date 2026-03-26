// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_HARMONY_XCOMPONENT_ADAPTER_H_
#define ANIMAX_SRC_BASE_UTIL_HARMONY_XCOMPONENT_ADAPTER_H_

#include <ace/xcomponent/native_interface_xcomponent.h>

#include <unordered_map>

namespace lynx {
namespace animax {

class XComponentCallback {
 public:
  virtual ~XComponentCallback() = default;

  virtual void OnSurfaceCreated(OH_NativeXComponent *component,
                                void *window) = 0;
  virtual void OnSurfaceChanged(OH_NativeXComponent *component,
                                void *window) = 0;
  virtual void OnSurfaceDestroyed(OH_NativeXComponent *component,
                                  void *window) = 0;
};

class XComponentAdapter {
 public:
  static XComponentAdapter &GetInstance();

  void RegisterCallback(OH_NativeXComponent *xcomponent,
                        XComponentCallback *callback);
  void UnregisterCallback(OH_NativeXComponent *xcomponent);

  void OnSurfaceCreated(OH_NativeXComponent *component, void *window);
  void OnSurfaceChanged(OH_NativeXComponent *component, void *window);
  void OnSurfaceDestroyed(OH_NativeXComponent *component, void *window);

 private:
  XComponentAdapter();

  OH_NativeXComponent_Callback callback_;
  std::unordered_map<OH_NativeXComponent *, XComponentCallback *> callback_map_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_HARMONY_XCOMPONENT_ADAPTER_H_
