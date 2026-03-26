// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_CLOSEABLE_BITMAP_REFERENCE_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_CLOSEABLE_BITMAP_REFERENCE_H_

#include "base/include/platform/android/jni_utils.h"
#include "base/include/platform/android/scoped_java_ref.h"

namespace lynx {
namespace animax {

class AnimaXCloseableBitmapReference {
 public:
  AnimaXCloseableBitmapReference(
      base::android::ScopedLocalJavaRef<jobject> ref);
  base::android::ScopedLocalJavaRef<jobject> GetBitmap();
  bool IsValid();
  void Close();

 private:
  JNIEnv* env_;
  base::android::ScopedLocalJavaRef<jobject> ref_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_CLOSEABLE_BITMAP_REFERENCE_H_
