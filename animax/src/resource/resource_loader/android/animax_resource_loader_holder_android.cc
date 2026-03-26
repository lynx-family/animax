// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/android/animax_resource_loader_holder_android.h"

#include "include/resource/uri_info.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXResourceLoaderHolder_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXResourceLoaderHolder_register_jni.h"

static jlong Create(JNIEnv* env, jclass jcaller) {
  auto* holder = new lynx::animax::AnimaXResourceLoaderHolderAndroid();
  return reinterpret_cast<jlong>(holder);
}

static void Destroy(JNIEnv* env, jclass jcaller, jlong native_holder) {
  if (native_holder) {
    auto* holder =
        reinterpret_cast<lynx::animax::AnimaXResourceLoaderHolderAndroid*>(
            native_holder);
    delete holder;
  }
}

static void RegisterLoaderForScheme(JNIEnv* env, jclass jcaller,
                                    jlong native_holder, jobject loader,
                                    jint scheme) {
  auto* holder_ptr =
      reinterpret_cast<lynx::animax::AnimaXResourceLoaderHolderAndroid*>(
          native_holder);
  auto scoped_loader =
      lynx::base::android::ScopedLocalJavaRef<jobject>(env, loader);
  if (holder_ptr && holder_ptr->resource_loader && !scoped_loader.IsNull()) {
    holder_ptr->resource_loader->RegisterAnimaXLoaderInterfaceForScheme(
        scoped_loader, static_cast<lynx::animax::UriInfo::Scheme>(scheme));
  }
}

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXResourceLoaderHolder(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

AnimaXResourceLoaderHolderAndroid::AnimaXResourceLoaderHolderAndroid() {
  resource_loader = std::make_shared<ResourcePipelineAndroid>();
  unzip_loader = AnimaxUnzipLoaderAndroid::Make<AnimaxUnzipLoaderAndroid>();
}

}  // namespace animax
}  // namespace lynx
