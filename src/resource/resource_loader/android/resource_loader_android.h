// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_RESOURCE_LOADER_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_RESOURCE_LOADER_ANDROID_H_

#include <jni.h>

#include "src/resource/resource_loader.h"

namespace lynx {
namespace animax {

std::unique_ptr<lynx::animax::Bitmap> JavaBitmapToBitmap(JNIEnv* env,
                                                         jobject java_bitmap);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_RESOURCE_LOADER_ANDROID_H_
