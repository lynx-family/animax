// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "platform/harmony/animax_lynx/src/main/cpp/registry/animax_registry.h"

#include <dlfcn.h>

#include <mutex>

#include "platform/harmony/lynx_harmony/src/main/cpp/lynx_context.h"
#include "platform/harmony/lynx_harmony/src/main/cpp/ui/ui_owner.h"

namespace lynx {
namespace tasm {
namespace harmony {

namespace {

void* TryGetAnimaXShardObjectHandler() {
  static void* animax_shard_object_handler = nullptr;
  static std::once_flag once_flag;
  std::call_once(once_flag, []() {
    void* handle = dlopen("liblynx_animax.so", RTLD_NOW | RTLD_GLOBAL);
    animax_shard_object_handler = handle;
  });

  return animax_shard_object_handler;
}

UIOwner::UICreatorFunc TryGetUIAnimaXCreatorFunc() {
  static UIOwner::UICreatorFunc ui_animax_creator_func = nullptr;
  static std::once_flag once_flag;
  std::call_once(once_flag, []() {
    void* handle = TryGetAnimaXShardObjectHandler();
    if (handle) {
      ui_animax_creator_func = reinterpret_cast<UIOwner::UICreatorFunc>(
          dlsym(handle, "UIAnimaXCreateFunc"));
    }
  });

  return ui_animax_creator_func;
}

void TryRegisterALogFunction() {
  static std::once_flag once_flag;
  std::call_once(once_flag, []() {
    void* handle = TryGetAnimaXShardObjectHandler();
    if (handle) {
      using RegisterFunc = void (*)();
      RegisterFunc register_a_log_func = reinterpret_cast<RegisterFunc>(
          dlsym(handle, "TryRegisterALogFunction"));
      if (register_a_log_func) {
        register_a_log_func();
      }
    }
  });
}

void TryLoadAnimaXNapi() {
  static void* animax_napi_shard_object_handler = nullptr;
  static std::once_flag once_flag;
  std::call_once(once_flag, []() {
    void* handle = dlopen("libanimax_napi.so", RTLD_NOW | RTLD_GLOBAL);
    animax_napi_shard_object_handler = handle;
  });
}

void TryLoadAnimaXTextra() {
  static void* animax_textra_shard_object_handler = nullptr;
  static std::once_flag once_flag;
  std::call_once(once_flag, []() {
    void* handle = dlopen("libanimax_textra.so", RTLD_NOW | RTLD_GLOBAL);
    animax_textra_shard_object_handler = handle;
  });
}

}  // namespace

void AnimaXRegistry::Init() {
  TryLoadAnimaXNapi();
  TryLoadAnimaXTextra();
  TryRegisterALogFunction();
  auto func = TryGetUIAnimaXCreatorFunc();
  if (func) {
    LynxContext::NodeInfo node_info{
        .ui_creator = func,
        .layout_node_creator = nullptr,
        .node_type = LayoutNodeType::COMMON,
    };
    auto& map = LynxContext::GetCAPINodeInfoMap();

    map["animax-view"] = node_info;
    map["lottie-view"] = node_info;
    map["x-alpha-video"] = node_info;
  }
}

}  // namespace harmony
}  // namespace tasm
}  // namespace lynx
