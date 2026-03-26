// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/animax_loader_completion_handler_android.h"

#include "platform/android/animax_android/src/main/jni/gen/AnimaXLoaderCompletionHandler_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXLoaderCompletionHandler_register_jni.h"
#include "src/base/log/log.h"
#include "src/resource/resource_loader/android/animax_loader_response_android.h"

namespace lynx {
namespace animax {

namespace {
struct CallbackAndRequestType {
  LoaderWrapperAndroid::CallbackType callback;
  ResourceRequestType request_type;
};
}  // namespace

base::android::ScopedGlobalJavaRef<jobject>
LoaderWrapperAndroidCallbackToAnimaXLoaderCompletionHandler(
    JNIEnv* env, ResourceRequestType request_type,
    LoaderWrapperAndroid::CallbackType callback) {
  const auto* callback_and_request_type = new CallbackAndRequestType{
      .callback = std::move(callback),
      .request_type = request_type,
  };
  auto completion_handler =
      Java_AnimaXLoaderCompletionHandler_createCompletionHandler(
          env, reinterpret_cast<jlong>(callback_and_request_type));
  return completion_handler;
}

}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXLoaderCompletionHandler(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax

static void OnComplete(JNIEnv* env, jobject jcaller, jlong native_ptr,
                       jobject loader_response) {
  auto callback_and_request_type =
      std::unique_ptr<lynx::animax::CallbackAndRequestType>(
          reinterpret_cast<lynx::animax::CallbackAndRequestType*>(native_ptr));

  if (!callback_and_request_type) {
    ANIMAX_LOGE("AnimaXLoaderCompletionHandler received a nullptr");
    return;
  }

  auto& callback = callback_and_request_type->callback;

  if (!callback) {
    ANIMAX_LOGE(
        "AnimaXLoaderCompletionHandler received an onComplete with null "
        "callback.");
    return;
  }

  auto loader_error =
      lynx::animax::AnimaXLoaderResponseToLoaderError(env, loader_response);
  if (loader_error) {
    callback({}, std::move(loader_error));
  } else {
    auto request_type = callback_and_request_type->request_type;
    auto transform_request = AnimaXLoaderResponseToTransformRequestAndroid(
        env, static_cast<lynx::animax::ResourceRequestType>(request_type),
        loader_response);
    callback(std::move(transform_request), {});
  }
}
