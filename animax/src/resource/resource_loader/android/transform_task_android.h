// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_TRANSFORM_TASK_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_TRANSFORM_TASK_ANDROID_H_

#include "base/include/platform/android/scoped_java_ref.h"
#include "include/resource/loader.h"
#include "include/resource/resource_task.h"

namespace lynx {
namespace animax {
enum class TransformRequestInputTypeAndroid {
  kJStringFilePath = 0,
  kJByteArray,
  kJAnimaXCloseableBitmapReference,
  kEnumCount,
  kInvalid = kEnumCount,
};

struct TransformRequestAndroid {
  // This ScopedGlobalJavaRef is safe with respect to cyclic references.
  // It only holds references to instances of String, Bitmap, or byte[].
  base::android::ScopedGlobalJavaRef<jobject> input{};
  TransformRequestInputTypeAndroid input_type =
      TransformRequestInputTypeAndroid::kInvalid;
  ResourcePayloadType output_type = ResourcePayloadType::kInvalid;
};

using TransformResponseAndroid = ResourceResponse;
using TransformLoaderBaseAndroid =
    Loader<TransformRequestAndroid, TransformResponseAndroid>;

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_TRANSFORM_TASK_ANDROID_H_
