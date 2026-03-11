// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_TRANSFORM_LOADER_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_TRANSFORM_LOADER_ANDROID_H_

#include <jni.h>

#include <array>

#include "base/include/platform/android/scoped_java_ref.h"
#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/resource/resource_loader/android/transform_task_android.h"

namespace lynx {
namespace animax {

class TransformLoaderAndroid : public TransformLoaderBaseAndroid {
 public:
  using TransformMatrix = std::array<
      std::array<Ptr, static_cast<size_t>(ResourcePayloadType::kEnumCount)>,
      static_cast<size_t>(TransformRequestInputTypeAndroid::kEnumCount)>;
  void Load(TransformRequestAndroid, CallbackType) override;
  ~TransformLoaderAndroid() override = default;
  TransformLoaderAndroid();
  TransformLoaderAndroid::Ptr GetLoader(
      TransformRequestInputTypeAndroid input_type,
      ResourcePayloadType output_type);

 private:
  TransformMatrix matrix_;
};

class JStringToNativeRawDataTransformLoader
    : public TransformLoaderBaseAndroid {
 public:
  void Load(TransformRequestAndroid, CallbackType) override;
  ~JStringToNativeRawDataTransformLoader() override = default;
};

class JStringToNativeStringTransformLoader : public TransformLoaderBaseAndroid {
 public:
  void Load(TransformRequestAndroid, CallbackType) override;
  ~JStringToNativeStringTransformLoader() override = default;
};

class JBitmapToNativeBitmapTransformLoader : public TransformLoaderBaseAndroid {
 public:
  void Load(TransformRequestAndroid, CallbackType) override;
  ~JBitmapToNativeBitmapTransformLoader() override = default;
};

class JByteArrayToNativeStringTransformLoader
    : public TransformLoaderBaseAndroid {
 public:
  void Load(TransformRequestAndroid, CallbackType) override;
  ~JByteArrayToNativeStringTransformLoader() override = default;
};

class JByteArrayToNativeRawDataTransformLoader
    : public TransformLoaderBaseAndroid {
 public:
  void Load(TransformRequestAndroid, CallbackType) override;
  ~JByteArrayToNativeRawDataTransformLoader() override = default;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_TRANSFORM_LOADER_ANDROID_H_
