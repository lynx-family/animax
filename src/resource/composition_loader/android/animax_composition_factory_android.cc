// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "base/include/platform/android/jni_convert_helper.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXCompositionFactory_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXCompositionFactory_register_jni.h"
#include "src/resource/composition_loader/android/animax_composition_callback_android.h"
#include "src/resource/composition_loader/animax_composition_factory.h"
#include "src/resource/resource_loader/android/animax_resource_loader_holder_android.h"

static void Create(JNIEnv* env, jobject jcaller, jstring scope,
                   jlong loader_holder) {
  auto* holder =
      reinterpret_cast<lynx::animax::AnimaXResourceLoaderHolderAndroid*>(
          loader_holder);
  if (holder) {
    auto scope_str =
        lynx::base::android::JNIConvertHelper::ConvertToString(env, scope);
    lynx::animax::AnimaXCompositionFactory::Instance().CreateLoader(
        std::move(scope_str), holder->resource_loader, holder->unzip_loader);
  }
}

static void Release(JNIEnv* env, jobject jcaller) {
  lynx::animax::AnimaXCompositionFactory::Instance().Release();
}

static void LoadJson(JNIEnv* env, jobject jcaller, jstring scope, jstring json,
                     jfloat density, jobject callback) {
  auto scope_str =
      lynx::base::android::JNIConvertHelper::ConvertToString(env, scope);
  auto json_str =
      lynx::base::android::JNIConvertHelper::ConvertToString(env, json);
  auto android_callback =
      std::make_shared<lynx::animax::AnimaXCompositionCallbackAndroid>(
          env, callback);
  lynx::animax::AnimaXCompositionFactory::Instance().LoadJson(
      std::move(scope_str), std::move(json_str), density,
      [android_callback = std::move(android_callback)](
          std::shared_ptr<lynx::animax::CompositionModel> model,
          const std::string& error) mutable {
        lynx::base::android::ScopedLocalJavaRef<jobject> composition;
        if (model != nullptr && error.empty()) {
          composition = lynx::animax::AnimaXCompositionAndroid::Create(model);
        }
        android_callback->OnCompositionCallback(composition.Get(), error);
      });
}

static void LoadUri(JNIEnv* env, jobject jcaller, jstring scope, jstring uri,
                    jfloat density, jobject callback) {
  auto scope_str =
      lynx::base::android::JNIConvertHelper::ConvertToString(env, scope);
  auto uri_str =
      lynx::base::android::JNIConvertHelper::ConvertToString(env, uri);
  auto android_callback =
      std::make_shared<lynx::animax::AnimaXCompositionCallbackAndroid>(
          env, callback);
  lynx::animax::AnimaXCompositionFactory::Instance().LoadUri(
      std::move(scope_str), std::move(uri_str), density,
      [android_callback = std::move(android_callback)](
          std::shared_ptr<lynx::animax::CompositionModel> model,
          const std::string& error) mutable {
        lynx::base::android::ScopedLocalJavaRef<jobject> composition;
        if (model != nullptr && error.empty()) {
          composition = lynx::animax::AnimaXCompositionAndroid::Create(model);
        }
        android_callback->OnCompositionCallback(composition.Get(), error);
      });
}

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXCompositionFactory(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {}  // namespace animax
}  // namespace lynx
