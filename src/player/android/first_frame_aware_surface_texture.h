// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_FIRST_FRAME_AWARE_SURFACE_TEXTURE_H_
#define ANIMAX_SRC_PLAYER_ANDROID_FIRST_FRAME_AWARE_SURFACE_TEXTURE_H_

#include "base/include/platform/android/scoped_java_ref.h"

namespace lynx {
namespace animax {

class SurfaceTextureAndroid {
 public:
  SurfaceTextureAndroid(JNIEnv *env, jobject surface_texture);
  virtual ~SurfaceTextureAndroid() = default;

  virtual void Release();

  SurfaceTextureAndroid(const SurfaceTextureAndroid &) = delete;
  SurfaceTextureAndroid &operator=(const SurfaceTextureAndroid &) = delete;

  SurfaceTextureAndroid(SurfaceTextureAndroid &&other) noexcept = default;
  SurfaceTextureAndroid &operator=(SurfaceTextureAndroid &&other) noexcept =
      default;

 protected:
  base::android::ScopedGlobalJavaRef<jobject> surface_texture_;
};

class FirstFrameAwareSurfaceTextureAndroid : public SurfaceTextureAndroid {
 public:
  void Release() override;

  FirstFrameAwareSurfaceTextureAndroid(JNIEnv *env, jobject surface_texture);
  ~FirstFrameAwareSurfaceTextureAndroid();

  FirstFrameAwareSurfaceTextureAndroid(
      const FirstFrameAwareSurfaceTextureAndroid &) = delete;
  FirstFrameAwareSurfaceTextureAndroid &operator=(
      const FirstFrameAwareSurfaceTextureAndroid &) = delete;

  FirstFrameAwareSurfaceTextureAndroid(
      FirstFrameAwareSurfaceTextureAndroid &&other) noexcept = default;
  FirstFrameAwareSurfaceTextureAndroid &operator=(
      FirstFrameAwareSurfaceTextureAndroid &&other) noexcept = default;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_FIRST_FRAME_AWARE_SURFACE_TEXTURE_H_
