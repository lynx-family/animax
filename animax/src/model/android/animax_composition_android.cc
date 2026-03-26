// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/model/android/animax_composition_android.h"

#include <memory>

#include "platform/android/animax_android/src/main/jni/gen/AnimaXComposition_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXComposition_register_jni.h"
#include "src/base/log/log.h"

static jlong GetStartFrame(JNIEnv* env, jobject jcaller, jlong native_ptr) {
  DCHECK(native_ptr);
  auto* ptr_holder =
      reinterpret_cast<std::shared_ptr<lynx::animax::CompositionModel>*>(
          native_ptr);
  return ptr_holder->get()->GetStartFrame();
}

static jlong GetEndFrame(JNIEnv* env, jobject jcaller, jlong native_ptr) {
  DCHECK(native_ptr);
  auto* ptr_holder =
      reinterpret_cast<std::shared_ptr<lynx::animax::CompositionModel>*>(
          native_ptr);
  return ptr_holder->get()->GetEndFrame();
}

static void Destroy(JNIEnv* env, jclass jcaller, jlong native_ptr) {
  DCHECK(native_ptr);
  auto* ptr_holder =
      reinterpret_cast<std::shared_ptr<lynx::animax::CompositionModel>*>(
          native_ptr);
  delete ptr_holder;
}

namespace animax {
namespace jni {

bool RegisterJNIForAnimaXComposition(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace jni
}  // namespace animax

namespace lynx {
namespace animax {

lynx::base::android::ScopedLocalJavaRef<jobject>
AnimaXCompositionAndroid::Create(
    std::shared_ptr<CompositionModel> composition) {
  JNIEnv* env = base::android::AttachCurrentThread();
  auto* ptr_holder =
      new std::shared_ptr<CompositionModel>(std::move(composition));
  jlong native_ptr = reinterpret_cast<jlong>(ptr_holder);
  return Java_AnimaXComposition_create(env, native_ptr);
}

}  // namespace animax
}  // namespace lynx
