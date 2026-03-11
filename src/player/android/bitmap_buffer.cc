// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/bitmap_buffer.h"

#include "android/bitmap.h"
#include "platform/android/animax_android/src/main/jni/gen/BitmapBuffer_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/BitmapBuffer_register_jni.h"
#include "src/base/log/log.h"
#include "src/base/util/android/hardware_buffer_functions.h"
#include "src/base/util/buffer_copy_helper.h"

namespace animax {
namespace jni {
bool RegisterJNIForBitmapBuffer(JNIEnv *env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

void BitmapBuffer::ResetBitmapWrapper(JNIEnv *env, jobject bitmap_wrapper) {
  bitmap_wrapper_.Reset(env, bitmap_wrapper);
}

bool BitmapBuffer::CopyTo(uint8_t *src, int32_t height, int32_t width) {
  if (HardwareBufferFunctions::IsWrappedBitmapSupported()) {
    return CopyToHardwareBuffer(src, height, width);
  } else {
    return CopyToBitmap(src, height, width);
  }
}

bool BitmapBuffer::CopyToBitmap(uint8_t *src, int32_t height, int32_t width) {
  SetState(BufferState::kPrepare);
  auto bitmap = GetBitmap();
  if (bitmap.IsNull()) {
    SetState(BufferState::kNotReady);
    return false;
  }

  JNIEnv *env = base::android::AttachCurrentThread();
  AndroidBitmapInfo info;
  if (AndroidBitmap_getInfo(env, bitmap.Get(), &info) !=
      ANDROID_BITMAP_RESULT_SUCCESS) {
    ANIMAX_LOGI("Failed to get bitmap info.")
    SetState(BufferState::kNotReady);
    return false;
  }
  if (!BufferCopyHelper::IsSizeMatch(width, height, info.width, info.height)) {
    SetState(BufferState::kNotReady);
    return false;
  }

  void *bitmap_buffer;
  if (AndroidBitmap_lockPixels(env, bitmap.Get(), &bitmap_buffer) !=
      ANDROID_BITMAP_RESULT_SUCCESS) {
    ANIMAX_LOGI("Failed to lock bitmap.")
    SetState(BufferState::kNotReady);
    return false;
  }
  BufferCopyHelper::CopyBuffer(src, reinterpret_cast<uint8_t *>(bitmap_buffer),
                               height, width * 4, info.stride);
  AndroidBitmap_unlockPixels(env, bitmap.Get());
  SetState(BufferState::kReady);
  return true;
}

bool BitmapBuffer::CopyToHardwareBuffer(uint8_t *src, int32_t height,
                                        int32_t width) {
  SetState(BufferState::kPrepare);
  JNIEnv *env = base::android::AttachCurrentThread();
  auto hardware_buffer_obj = GetHardwareBuffer();
  if (hardware_buffer_obj.IsNull()) {
    SetState(BufferState::kNotReady);
    return false;
  }

  AHardwareBuffer *hardware_buffer =
      HardwareBufferFunctions::GetInstance().FromHardwareBuffer(
          env, hardware_buffer_obj.Get());
  if (hardware_buffer == nullptr) {
    ANIMAX_LOGI("Failed to get AHardwareBuffer.")
    SetState(BufferState::kNotReady);
    return false;
  }

  AHardwareBuffer_Desc desc;
  HardwareBufferFunctions::GetInstance().Describe(hardware_buffer, &desc);
  if (!BufferCopyHelper::IsSizeMatch(width, height, desc.width, desc.height)) {
    SetState(BufferState::kNotReady);
    return false;
  }

  void *dst_buffer;
  int32_t fence = -1;
  if (HardwareBufferFunctions::GetInstance().Lock(
          hardware_buffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, -1, nullptr,
          &dst_buffer) == 0) {
    BufferCopyHelper::CopyBuffer(src, reinterpret_cast<uint8_t *>(dst_buffer),
                                 height, width * 4, desc.stride * 4);
    HardwareBufferFunctions::GetInstance().Unlock(hardware_buffer, &fence);
    SetState(BufferState::kReady);
    return true;
  } else {
    HardwareBufferFunctions::GetInstance().Unlock(hardware_buffer, nullptr);
    ANIMAX_LOGI("Failed to lock hardware buffer.")
    SetState(BufferState::kNotReady);
    return false;
  }
}

void BitmapBuffer::SetState(BufferState state) {
  if (bitmap_wrapper_.IsNull()) {
    ANIMAX_LOGI("NoteIsOffscreenRendered but no ref.")
    return;
  }
  auto *env = base::android::AttachCurrentThread();
  Java_BitmapBuffer_setState(env, bitmap_wrapper_.Get(),
                             static_cast<int32_t>(state));
}

lynx::base::android::ScopedLocalJavaRef<jobject> BitmapBuffer::GetBitmap() {
  if (bitmap_wrapper_.IsNull()) {
    ANIMAX_LOGI("GetBitmap but no ref.")
    return base::android::ScopedLocalJavaRef<jobject>();
  }

  auto *env = base::android::AttachCurrentThread();
  return Java_BitmapBuffer_getBitmap(env, bitmap_wrapper_.Get());
}

lynx::base::android::ScopedLocalJavaRef<jobject>
BitmapBuffer::GetHardwareBuffer() {
  if (bitmap_wrapper_.IsNull()) {
    ANIMAX_LOGI("GetHardwareBuffer but no ref.")
    return base::android::ScopedLocalJavaRef<jobject>();
  }

  auto *env = base::android::AttachCurrentThread();
  return Java_BitmapBuffer_getHardwareBuffer(env, bitmap_wrapper_.Get());
}

}  // namespace animax
}  // namespace lynx
