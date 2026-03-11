// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_LOADER_COMPLETION_HANDLER_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_LOADER_COMPLETION_HANDLER_ANDROID_H_

#include "base/include/platform/android/jni_utils.h"
#include "base/include/platform/android/scoped_java_ref.h"
#include "src/resource/resource_loader/android/loader_wrapper_android.h"

namespace lynx {
namespace animax {

base::android::ScopedGlobalJavaRef<jobject>
LoaderWrapperAndroidCallbackToAnimaXLoaderCompletionHandler(
    JNIEnv* env, ResourceRequestType request_type,
    LoaderWrapperAndroid::CallbackType callback);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_LOADER_COMPLETION_HANDLER_ANDROID_H_
