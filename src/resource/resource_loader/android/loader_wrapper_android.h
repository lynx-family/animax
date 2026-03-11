// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_LOADER_WRAPPER_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_LOADER_WRAPPER_ANDROID_H_

#include <jni.h>

#include "base/include/platform/android/scoped_java_ref.h"
#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/resource/resource_loader/android/transform_task_android.h"

namespace lynx {
namespace animax {

class LoaderWrapperAndroid
    : public Loader<ResourceRequest, TransformRequestAndroid> {
 public:
  LoaderWrapperAndroid(base::android::ScopedLocalJavaRef<jobject> loader)
      : android_loader_{loader} {}

  ~LoaderWrapperAndroid() override = default;
  void Load(ResourceRequest request, CallbackType callback) override;

 private:
  base::android::ScopedLocalJavaRef<jobject> GetLoader() const;
  base::android::ScopedWeakGlobalJavaRef<jobject> android_loader_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_LOADER_WRAPPER_ANDROID_H_
