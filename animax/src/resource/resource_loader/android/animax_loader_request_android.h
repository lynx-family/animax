// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_LOADER_REQUEST_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_LOADER_REQUEST_ANDROID_H_

#include <jni.h>

#include "base/include/platform/android/jni_utils.h"
#include "include/resource/resource_task.h"

namespace lynx {
namespace animax {

base::android::ScopedLocalJavaRef<jobject> ResourceRequestToAnimaXLoaderRequest(
    JNIEnv* env, const ResourceRequest& resource_request);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_LOADER_REQUEST_ANDROID_H_
