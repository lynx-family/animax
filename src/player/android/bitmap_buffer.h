// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_BITMAP_BUFFER_H_
#define ANIMAX_SRC_PLAYER_ANDROID_BITMAP_BUFFER_H_

#include "base/include/platform/android/scoped_java_ref.h"

namespace lynx {
namespace animax {

class BitmapBuffer {
 public:
  void ResetBitmapWrapper(JNIEnv* env, jobject bitmap_wrapper);
  bool CopyTo(uint8_t* src, int32_t height, int32_t width);

 private:
  base::android::ScopedGlobalJavaRef<jobject> bitmap_wrapper_;

  enum class BufferState {
    kNotReady,
    kPrepare,
    kReady,
    kDestroyed,
  };

  bool CopyToBitmap(uint8_t* src, int32_t height, int32_t width);
  bool CopyToHardwareBuffer(uint8_t* src, int32_t height, int32_t width);

  void SetState(BufferState state);
  lynx::base::android::ScopedLocalJavaRef<jobject> GetBitmap();
  lynx::base::android::ScopedLocalJavaRef<jobject> GetHardwareBuffer();
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_BITMAP_BUFFER_H_
