// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/android/hardware_buffer_functions.h"

#include <dlfcn.h>
#include <sys/system_properties.h>

#include "src/base/log/log.h"
#include "src/base/util/android/symbol_util.h"

namespace lynx {
namespace animax {

namespace {
void *GetAndroidSoHandle() {
  static void *handle =
      dlopen("libandroid.so", RTLD_NOW | RTLD_NODELETE | RTLD_LOCAL);
  if (!handle) {
    ANIMAX_LOGE("Failed to open libandroid.so for AHardwareBuffer!")
  }
  return handle;
}

int GetSDKVersion() {
  static const int sdk_version = []() {
    char sdk_prop[PROP_VALUE_MAX] = "0";
    __system_property_get("ro.build.version.sdk", sdk_prop);
    return std::atoi(sdk_prop);
  }();
  return sdk_version;
}
}  // namespace

HardwareBufferFunctions::HardwareBufferFunctions() {
  // todo(lixianruo.cyrus): Or IsGLHardwareBufferSupported.
  if (IsWrappedBitmapSupported()) {
    void *handle = GetAndroidSoHandle();
    SymbolUtil::LoadSymbol(handle, describe_, "AHardwareBuffer_describe");
    SymbolUtil::LoadSymbol(handle, lock_, "AHardwareBuffer_lock");
    SymbolUtil::LoadSymbol(handle, unlock_, "AHardwareBuffer_unlock");
    SymbolUtil::LoadSymbol(handle, from_hardware_buffer_,
                           "AHardwareBuffer_fromHardwareBuffer");
  }
}

HardwareBufferFunctions &HardwareBufferFunctions::GetInstance() {
  static lynx::base::NoDestructor<HardwareBufferFunctions> functions;
  return *functions;
}

bool HardwareBufferFunctions::IsWrappedBitmapSupported() {
  return GetSDKVersion() >= 29;
}

void HardwareBufferFunctions::Describe(const AHardwareBuffer *buffer,
                                       AHardwareBuffer_Desc *outDesc) {
  DCHECK(describe_);
  describe_(buffer, outDesc);
}

int HardwareBufferFunctions::Lock(AHardwareBuffer *buffer, uint64_t usage,
                                  int32_t fence, const ARect *rect,
                                  void **out_virtual_address) {
  DCHECK(lock_);
  return lock_(buffer, usage, fence, rect, out_virtual_address);
}

int HardwareBufferFunctions::Unlock(AHardwareBuffer *buffer, int32_t *fence) {
  DCHECK(unlock_);
  return unlock_(buffer, fence);
}

AHardwareBuffer *HardwareBufferFunctions::FromHardwareBuffer(
    JNIEnv *env, jobject hardware_buffer_obj) {
  DCHECK(from_hardware_buffer_);
  return from_hardware_buffer_(env, hardware_buffer_obj);
}
}  // namespace animax
}  // namespace lynx
