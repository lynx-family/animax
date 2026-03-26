// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_BITMAP_BUFFER_GROUP_H_
#define ANIMAX_SRC_PLAYER_ANDROID_BITMAP_BUFFER_GROUP_H_

#include "base/include/platform/android/scoped_java_ref.h"
#include "src/player/android/bitmap_buffer.h"

namespace lynx {
namespace animax {

class BitmapBufferGroup {
 public:
  BitmapBufferGroup(JNIEnv* env, jobject bitmap_buffer);

  void CopyTo(uint8_t* src, int32_t height, int32_t width);

 private:
  base::android::ScopedGlobalJavaRef<jobject> bitmap_buffer_group_;
  std::unique_ptr<BitmapBuffer> bitmap_wrapper_ =
      std::make_unique<BitmapBuffer>();
  lynx::base::android::ScopedLocalJavaRef<jobject>
  GetBitmapWrapperForOffscreenRendering();
  void NotifyBufferUpdate();
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_BITMAP_BUFFER_GROUP_H_
