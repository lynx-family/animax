// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_RESOURCE_PIPELINE_ANDROID_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_RESOURCE_PIPELINE_ANDROID_H_

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/resource/resource_loader/android/loader_wrapper_android.h"

namespace lynx {
namespace animax {

class ResourcePipelineAndroid
    : public Loader<ResourceRequest, ResourceResponse> {
 public:
  ResourcePipelineAndroid();
  ~ResourcePipelineAndroid() override = default;
  void Load(ResourceRequest request, CallbackType callback) override;
  void RegisterAnimaXLoaderInterfaceForScheme(
      base::android::ScopedLocalJavaRef<jobject> loader,
      UriInfo::Scheme scheme);

 private:
  Loader<ResourceRequest, TransformRequestAndroid>::Ptr platform_stage_{};
  Loader<ResourceRequest, ResourceResponse>::Ptr pipeline_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_ANDROID_RESOURCE_PIPELINE_ANDROID_H_
