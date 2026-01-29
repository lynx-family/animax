// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_CONTEXT_ANDROID_H_
#define ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_CONTEXT_ANDROID_H_

#include <jni.h>

#include "base/include/platform/android/scoped_java_ref.h"

namespace lynx {
namespace animax {

class AnimaXContextAndroid {
 public:
  explicit AnimaXContextAndroid(JNIEnv* env, jobject java_context);
  ~AnimaXContextAndroid();

  AnimaXContextAndroid(const AnimaXContextAndroid&) = delete;
  AnimaXContextAndroid& operator=(const AnimaXContextAndroid&) = delete;

  AnimaXContextAndroid(AnimaXContextAndroid&&) = default;
  AnimaXContextAndroid& operator=(AnimaXContextAndroid&&) = default;

  base::android::ScopedLocalJavaRef<jobject> GetAbility() const;
  bool MultiThreadAccelerate() const;
  bool DisablePlaybackOnAssetLoadFailure() const;
  float GetDeviceDensity() const;

 private:
  base::android::ScopedGlobalJavaRef<jobject> java_context_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_ANIMAX_CONTEXT_ANDROID_H_
