// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/vk/context_vk.h"

#include <dlfcn.h>

#include <vector>

#if defined(OS_ANDROID) && !defined(VK_USE_PLATFORM_ANDROID_KHR)
#define VK_USE_PLATFORM_ANDROID_KHR 1
#endif

#include "skity/gpu/gpu_context_vk.hpp"
#include "src/render/error_callback.h"

namespace lynx {
namespace animax {

namespace {
PFN_vkGetInstanceProcAddr LoadVkGetInstanceProcAddr() {
  // Try to load the Vulkan shared library
  void *vulkan_lib = dlopen("libvulkan.so", RTLD_NOW);
  if (!vulkan_lib) {
    return nullptr;
  }

  auto proc_addr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      dlsym(vulkan_lib, "vkGetInstanceProcAddr"));

  if (!proc_addr) {
    dlclose(vulkan_lib);
  }

  return proc_addr;
}

std::vector<const char *> GetRequiredInstanceExtensions() {
  return {
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
  };
}

std::vector<const char *> GetRequiredDeviceExtensions() {
  return {
      VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME,
  };
}

std::unique_ptr<skity::GPUContext> CreateSkityVkContext() {
  if (!skity::IsGPUBackendSupported(skity::GPUBackendType::kVulkan)) {
    // the skity library does not enable vulkan backend at runtime
    return {};
  }

  auto get_instance_proc_addr = LoadVkGetInstanceProcAddr();
  if (!get_instance_proc_addr) {
    // failed to load vkGetInstanceProcAddr via dlopen
    return {};
  }

  auto extensions = GetRequiredInstanceExtensions();
  auto device_extensions = GetRequiredDeviceExtensions();

  skity::GPUContextInfoVK info = {};
  info.get_instance_proc_addr = get_instance_proc_addr;
  info.enabled_instance_extensions = extensions.data();
  info.enabled_instance_extension_count =
      static_cast<uint32_t>(extensions.size());
  info.enabled_device_extensions = device_extensions.data();
  info.enabled_device_extension_count =
      static_cast<uint32_t>(device_extensions.size());

  auto ctx = skity::CreateGPUContextVK(&info);

  if (ctx) {
    ctx->SetErrorCallback(&SkityErrorCallbackDefault, nullptr);
    ctx->SetEnableTextLinearFilter(true);
  }

  return ctx;
}
}  // namespace

std::shared_ptr<skity::GPUContext> ContextVk::GetGPUContext() {
  static thread_local std::shared_ptr<skity::GPUContext> ctx =
      CreateSkityVkContext();

  return ctx;
}

}  // namespace animax
}  // namespace lynx
