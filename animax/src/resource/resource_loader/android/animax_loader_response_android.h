// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_LOADER_RESPONSE_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_LOADER_RESPONSE_ANDROID_H_

#include <jni.h>

#include "include/resource/loader_error.h"
#include "include/resource/resource_task.h"
#include "src/resource/resource_loader/android/transform_task_android.h"

namespace lynx {
namespace animax {

LoaderError AnimaXLoaderResponseToLoaderError(JNIEnv* env, jobject response);
TransformRequestAndroid AnimaXLoaderResponseToTransformRequestAndroid(
    JNIEnv* env, ResourceRequestType request_type, jobject response);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_LOADER_RESPONSE_ANDROID_H_
