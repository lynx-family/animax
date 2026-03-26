// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/animax_closeable_bitmap_reference.h"

#include "platform/android/animax_android/src/main/jni/gen/AnimaXCloseableBitmapReferenceNativeAdapter_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXCloseableBitmapReferenceNativeAdapter_register_jni.h"

namespace lynx {
namespace animax {

AnimaXCloseableBitmapReference::AnimaXCloseableBitmapReference(
    base::android::ScopedLocalJavaRef<jobject> ref)
    : env_{base::android::AttachCurrentThread()}, ref_{ref} {}

base::android::ScopedLocalJavaRef<jobject>
AnimaXCloseableBitmapReference::GetBitmap() {
  return base::android::ScopedLocalJavaRef<jobject>(
      Java_AnimaXCloseableBitmapReferenceNativeAdapter_get(env_, ref_.Get()));
}
bool AnimaXCloseableBitmapReference::IsValid() {
  return Java_AnimaXCloseableBitmapReferenceNativeAdapter_isValid(env_,
                                                                  ref_.Get());
}
void AnimaXCloseableBitmapReference::Close() {
  return Java_AnimaXCloseableBitmapReferenceNativeAdapter_close(env_,
                                                                ref_.Get());
}

}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXCloseableBitmapReferenceNativeAdapter(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
