// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_SURFACE_DRAWABLE_ANDROID_H_
#define ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_SURFACE_DRAWABLE_ANDROID_H_

#include <jni.h>

#include <memory>

#include "base/include/platform/android/scoped_java_ref.h"

namespace lynx {
namespace animax {

class AnimaXSurfaceDrawableAndroid {
 public:
  enum class Type {
    kInvalid,
    kTextureView,        // Surface + SurfaceTexture
    kSurfaceView,        // Surface only
    kBitmapBufferGroup,  // ImageView
  };

  explicit AnimaXSurfaceDrawableAndroid(JNIEnv* env, jobject java_surface);
  ~AnimaXSurfaceDrawableAndroid();

  // Disable copy
  AnimaXSurfaceDrawableAndroid(const AnimaXSurfaceDrawableAndroid&) = delete;
  AnimaXSurfaceDrawableAndroid& operator=(const AnimaXSurfaceDrawableAndroid&) =
      delete;

  // Enable move
  AnimaXSurfaceDrawableAndroid(AnimaXSurfaceDrawableAndroid&&) = default;
  AnimaXSurfaceDrawableAndroid& operator=(AnimaXSurfaceDrawableAndroid&&) =
      default;

  Type GetType() const;

  // Get the underlying objects
  base::android::ScopedLocalJavaRef<jobject> GetSurface() const;
  base::android::ScopedLocalJavaRef<jobject> GetSurfaceTexture() const;
  base::android::ScopedLocalJavaRef<jobject> GetBitmapBufferGroup() const;
  bool IsTextureFirstFrameAware() const;

  int32_t GetWidth() const;
  int32_t GetHeight() const;

  // Check if the wrapper is valid
  bool IsValid() const { return java_surface_.Get() != nullptr; }

  bool HasValidDrawable() const {
    return (!GetSurface().IsNull() || !GetBitmapBufferGroup().IsNull()) &&
           GetWidth() > 0 && GetHeight() > 0;
  }

  // Properties passed down from Android
  bool IsAntiAliasingEnabled() const;
  bool IsSoftwareRenderEnabled() const;
  bool IsPlatformSurfaceInitiallyInvalid() const;
  bool IsAutoDestroyEGLContextEnabled() const;

 private:
  // Strong reference to Java AnimaXSurfaceWrapper object
  base::android::ScopedGlobalJavaRef<jobject> java_surface_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_SURFACE_DRAWABLE_ANDROID_H_
