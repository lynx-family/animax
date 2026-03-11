// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/resource/resource_loader/android/resource_pipeline_android.h"

#include <array>

#include "src/resource/loader/exec_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/resource_loader/android/transform_loader_android.h"

namespace lynx {
namespace animax {

namespace {
using PlatformLoader = Loader<ResourceRequest, TransformRequestAndroid>;

class SchemeRouterLoader : public PlatformLoader {
 public:
  static std::shared_ptr<SchemeRouterLoader> MakeRouter() {
    return std::shared_ptr<SchemeRouterLoader>(new SchemeRouterLoader{});
  }

  ~SchemeRouterLoader() override = default;
  void RegisterLoaderForScheme(UriInfo::Scheme scheme,
                               PlatformLoader::Ptr ptr) {
    loaders_[static_cast<std::size_t>(scheme)] = ptr;
  }

  PlatformLoader::Ptr LoaderForScheme(UriInfo::Scheme scheme) {
    return loaders_[static_cast<std::size_t>(scheme)];
  }

  void Load(ResourceRequest request, CallbackType callback) override {
    auto loader = LoaderForScheme(request.uri_info.scheme);
    if (!loader) {
      callback(
          TransformRequestAndroid{},
          LoaderError{
              .code = kNoSuchLoader,
              .message = "No such loader for handling UriInfo scheme type: " +
                         std::to_string(
                             static_cast<int32_t>(request.uri_info.scheme))});
      return;
    }
    loader->Load(std::move(request), std::move(callback));
  }

 private:
  SchemeRouterLoader() = default;
  std::array<PlatformLoader::Ptr,
             static_cast<std::size_t>(UriInfo::Scheme::kEnumCount)>
      loaders_{};
};
}  // namespace

ResourcePipelineAndroid::ResourcePipelineAndroid() {
  auto scheme_router = SchemeRouterLoader::MakeRouter();
  platform_stage_ = scheme_router;
  auto transform_stage = TransformLoaderAndroid::Make<TransformLoaderAndroid>();
  pipeline_ = platform_stage_ | transform_stage;
}

void ResourcePipelineAndroid::RegisterAnimaXLoaderInterfaceForScheme(
    base::android::ScopedLocalJavaRef<jobject> loader, UriInfo::Scheme scheme) {
  auto scheme_router =
      std::static_pointer_cast<SchemeRouterLoader>(platform_stage_);
  auto new_loader = PlatformLoader::Make<LoaderWrapperAndroid>(loader);
  scheme_router->RegisterLoaderForScheme(scheme, new_loader);
}

void ResourcePipelineAndroid::Load(ResourceRequest request,
                                   CallbackType callback) {
  pipeline_->Load(std::move(request), std::move(callback));
}

}  // namespace animax
}  // namespace lynx
