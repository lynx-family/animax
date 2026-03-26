// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/bitmap_buffer_group.h"

#include <cstring>

#include "platform/android/animax_android/src/main/jni/gen/BitmapBufferGroup_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/BitmapBufferGroup_register_jni.h"
#include "src/base/log/log.h"
#include "src/player/android/bitmap_buffer.h"

namespace animax {
namespace jni {

bool RegisterJNIForBitmapBufferGroup(JNIEnv *env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

BitmapBufferGroup::BitmapBufferGroup(JNIEnv *env, jobject bitmap_buffer) {
  bitmap_buffer_group_.Reset(env, bitmap_buffer);
}

void BitmapBufferGroup::NotifyBufferUpdate() {
  if (bitmap_buffer_group_.IsNull()) {
    ANIMAX_LOGI("NotifyBufferUpdate but no ref.")
    return;
  }

  auto *env = base::android::AttachCurrentThread();
  Java_BitmapBufferGroup_onBufferUpdate(env, bitmap_buffer_group_.Get());
}

base::android::ScopedLocalJavaRef<jobject>
BitmapBufferGroup::GetBitmapWrapperForOffscreenRendering() {
  if (bitmap_buffer_group_.IsNull()) {
    ANIMAX_LOGI("GetBitmapWrapperForOffscreenRendering but no ref.")
    return base::android::ScopedLocalJavaRef<jobject>();
  }

  auto *env = base::android::AttachCurrentThread();
  return Java_BitmapBufferGroup_getBitmapBufferForOffscreenRendering(
      env, bitmap_buffer_group_.Get());
}

void BitmapBufferGroup::CopyTo(uint8_t *src, int32_t height, int32_t width) {
  auto *env = base::android::AttachCurrentThread();
  auto java_wrapper = GetBitmapWrapperForOffscreenRendering();
  if (java_wrapper.IsNull()) {
    ANIMAX_LOGI("CopyToBitmapBuffer but no ref.")
    return;
  }
  bitmap_wrapper_->ResetBitmapWrapper(env, java_wrapper.Get());
  if (bitmap_wrapper_->CopyTo(src, height, width)) {
    NotifyBufferUpdate();
  }
}

}  // namespace animax
}  // namespace lynx
