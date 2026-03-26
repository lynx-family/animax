// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/loader_wrapper_android.h"

#include "base/include/platform/android/jni_utils.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXLoader_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXLoader_register_jni.h"
#include "src/resource/resource_loader/android/animax_loader_completion_handler_android.h"
#include "src/resource/resource_loader/android/animax_loader_request_android.h"
#include "src/resource/resource_loader/android/animax_loader_response_android.h"

namespace lynx {
namespace animax {

base::android::ScopedLocalJavaRef<jobject> LoaderWrapperAndroid::GetLoader()
    const {
  return base::android::ScopedLocalJavaRef<jobject>{android_loader_};
}

void LoaderWrapperAndroid::Load(ResourceRequest request,
                                CallbackType callback) {
  auto android_loader = GetLoader();

  if (android_loader.IsNull()) {
    callback({}, LoaderError{.code = kInvalidLoader,
                             .message = "AnimaXLoaderInterface is null."});
    return;
  }

  if (request.type == ResourceRequestType::kInvalid) {
    callback(
        {},
        LoaderError{
            .code = kInvalidLoaderRequest,
            .message = "LoaderWrapper received a request with invalid type."});
    return;
  }
  JNIEnv* env = base::android::AttachCurrentThread();
  auto animax_loader_request =
      ResourceRequestToAnimaXLoaderRequest(env, request);
  auto animax_completion_handler =
      LoaderWrapperAndroidCallbackToAnimaXLoaderCompletionHandler(
          env, request.type, std::move(callback));
  Java_AnimaXLoader_load(env, android_loader.Get(), animax_loader_request.Get(),
                         animax_completion_handler.Get());
}

}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXLoader(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
