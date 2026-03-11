// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/animax_surface_drawable_android.h"

#include "platform/android/animax_android/src/main/jni/gen/AnimaXSurfaceDrawable_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXSurfaceDrawable_register_jni.h"

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXSurfaceDrawable(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

AnimaXSurfaceDrawableAndroid::AnimaXSurfaceDrawableAndroid(JNIEnv* env,
                                                           jobject java_surface)
    : java_surface_(
          base::android::ScopedGlobalJavaRef<jobject>(env, java_surface)) {}

AnimaXSurfaceDrawableAndroid::~AnimaXSurfaceDrawableAndroid() = default;

AnimaXSurfaceDrawableAndroid::Type AnimaXSurfaceDrawableAndroid::GetType()
    const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return static_cast<AnimaXSurfaceDrawableAndroid::Type>(
      Java_AnimaXSurfaceDrawable_getTypeIndex(env, java_surface_.Get()));
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXSurfaceDrawableAndroid::GetSurface() const {
  if (!IsValid()) return base::android::ScopedLocalJavaRef<jobject>();
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXSurfaceDrawable_getSurface(env, java_surface_.Get());
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXSurfaceDrawableAndroid::GetSurfaceTexture() const {
  if (!IsValid()) return base::android::ScopedLocalJavaRef<jobject>();
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXSurfaceDrawable_getSurfaceTexture(env, java_surface_.Get());
}

base::android::ScopedLocalJavaRef<jobject>
AnimaXSurfaceDrawableAndroid::GetBitmapBufferGroup() const {
  if (!IsValid()) return base::android::ScopedLocalJavaRef<jobject>();
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXSurfaceDrawable_getBitmapBufferGroup(env,
                                                         java_surface_.Get());
}

bool AnimaXSurfaceDrawableAndroid::IsTextureFirstFrameAware() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXSurfaceDrawable_isTextureFirstFrameAware(
      env, java_surface_.Get());
}

int32_t AnimaXSurfaceDrawableAndroid::GetWidth() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXSurfaceDrawable_getWidth(env, java_surface_.Get());
}
int32_t AnimaXSurfaceDrawableAndroid::GetHeight() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXSurfaceDrawable_getHeight(env, java_surface_.Get());
}

bool AnimaXSurfaceDrawableAndroid::IsAntiAliasingEnabled() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXSurfaceDrawable_isAntiAliasingEnabled(env,
                                                          java_surface_.Get());
}
bool AnimaXSurfaceDrawableAndroid::IsSoftwareRenderEnabled() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXSurfaceDrawable_isSoftwareRenderEnabled(
      env, java_surface_.Get());
}
bool AnimaXSurfaceDrawableAndroid::IsPlatformSurfaceInitiallyInvalid() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXSurfaceDrawable_isPlatformSurfaceInitiallyInvalid(
      env, java_surface_.Get());
}
bool AnimaXSurfaceDrawableAndroid::IsAutoDestroyEGLContextEnabled() const {
  JNIEnv* env = base::android::AttachCurrentThread();
  return Java_AnimaXSurfaceDrawable_isAutoDestroyEGLContextEnabled(
      env, java_surface_.Get());
}

}  // namespace animax
}  // namespace lynx
