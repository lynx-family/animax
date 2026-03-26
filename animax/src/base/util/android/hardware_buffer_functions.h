// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_ANDROID_HARDWARE_BUFFER_FUNCTIONS_H_
#define ANIMAX_SRC_BASE_UTIL_ANDROID_HARDWARE_BUFFER_FUNCTIONS_H_

#include <android/hardware_buffer.h>
#include <jni.h>

#include "base/include/no_destructor.h"

namespace lynx {
namespace animax {

using DescribeFunc = void(const AHardwareBuffer* buffer,
                          AHardwareBuffer_Desc* outDesc);
using LockFunc = int(AHardwareBuffer* buffer, uint64_t usage, int32_t fence,
                     const ARect* rect, void** outVirtualAddress);
using UnlockFunc = int(AHardwareBuffer* buffer, int32_t* fence);
using FromHardwareBufferFunc = AHardwareBuffer*(JNIEnv* env,
                                                jobject hardwareBufferObj);

class HardwareBufferFunctions {
 public:
  static bool IsWrappedBitmapSupported();
  static HardwareBufferFunctions& GetInstance();

  HardwareBufferFunctions(const HardwareBufferFunctions&) = delete;
  HardwareBufferFunctions& operator=(const HardwareBufferFunctions&) = delete;

  void Describe(const AHardwareBuffer* buffer, AHardwareBuffer_Desc* outDesc);
  int Lock(AHardwareBuffer* buffer, uint64_t usage, int32_t fence,
           const ARect* rect, void** out_virtual_address);
  int Unlock(AHardwareBuffer* buffer, int32_t* fence);
  AHardwareBuffer* FromHardwareBuffer(JNIEnv* env, jobject hardware_buffer_obj);

 private:
  friend class lynx::base::NoDestructor<HardwareBufferFunctions>;
  HardwareBufferFunctions();
  DescribeFunc* describe_ = nullptr;
  LockFunc* lock_ = nullptr;
  UnlockFunc* unlock_ = nullptr;
  FromHardwareBufferFunc* from_hardware_buffer_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_ANDROID_HARDWARE_BUFFER_FUNCTIONS_H_
