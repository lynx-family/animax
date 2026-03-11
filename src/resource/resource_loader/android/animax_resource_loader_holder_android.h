// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_RESOURCE_LOADER_HOLDER_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_RESOURCE_LOADER_HOLDER_ANDROID_H_

#include <memory>

#include "base/include/platform/android/jni_utils.h"
#include "base/include/platform/android/scoped_java_ref.h"
#include "src/resource/resource_loader/android/animax_unzip_loader_android.h"
#include "src/resource/resource_loader/android/resource_pipeline_android.h"

namespace lynx {
namespace animax {

class AnimaXResourceLoaderHolderAndroid {
 public:
  AnimaXResourceLoaderHolderAndroid();
  ~AnimaXResourceLoaderHolderAndroid() = default;

  std::shared_ptr<ResourcePipelineAndroid> resource_loader{};
  Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_ANIMAX_RESOURCE_LOADER_HOLDER_ANDROID_H_
