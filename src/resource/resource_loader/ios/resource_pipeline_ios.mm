// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/ios/resource_pipeline_ios.h"
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXResourceFactoryService.h>
#include "src/resource/loader/exec_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/resource_loader/ios/loader_wrapper_ios.h"
#include "src/resource/resource_loader/ios/transform_loader_ios.h"

#import "AnimaXBase64Loader.h"
#import "AnimaXFileLoader.h"

#include <map>

namespace lynx {
namespace animax {

namespace {
using PlatformLoader = Loader<ResourceRequest, TransformRequestIOS>;

class SchemeRouterLoader : public PlatformLoader {
 public:
  static std::shared_ptr<SchemeRouterLoader> MakeRouter() {
    return std::shared_ptr<SchemeRouterLoader>(new SchemeRouterLoader{});
  }

  ~SchemeRouterLoader() = default;
  void RegisterLoaderForScheme(UriInfo::Scheme scheme, PlatformLoader::Ptr ptr) {
    loaders_[static_cast<int>(scheme)] = ptr;
  }

  PlatformLoader::Ptr LoaderForScheme(UriInfo::Scheme scheme) {
    return loaders_[static_cast<int>(scheme)];
  }

  void Load(ResourceRequest request, CallbackType callback) override {
    auto loader = LoaderForScheme(request.uri_info.scheme);
    if (!loader) {
      callback(TransformRequestIOS{},
               LoaderError{.code = kNoSuchLoader,
                           "No such loader for handling UriInfo scheme type: " +
                               std::to_string(static_cast<int32_t>(request.uri_info.scheme))});
      return;
    }
    loader->Load(std::move(request), std::move(callback));
  }

 private:
  std::array<PlatformLoader::Ptr, static_cast<std::size_t>(UriInfo::Scheme::kEnumCount)> loaders_;
};
}  // namespace

std::shared_ptr<ResourcePipelineIOS> ResourcePipelineIOS::MakePipeline(AnimaXContext *context) {
  return std::shared_ptr<ResourcePipelineIOS>{new ResourcePipelineIOS(context)};
}

ResourcePipelineIOS::ResourcePipelineIOS(AnimaXContext *context) {
  file_loader_ = [[AnimaXFileLoader alloc] init];
  auto file_loader = ResourceThreadExecLoader(PlatformLoader::Make<LoaderWrapperIOS>(file_loader_));
  auto base64_loader = ResourceThreadExecLoader(
      PlatformLoader::Make<LoaderWrapperIOS>([[AnimaXBase64Loader alloc] init]));

  auto scheme_router = SchemeRouterLoader::MakeRouter();
  scheme_router->RegisterLoaderForScheme(UriInfo::Scheme::kFile, file_loader);
  scheme_router->RegisterLoaderForScheme(UriInfo::Scheme::kDataURL, base64_loader);

  platform_stage_ = std::static_pointer_cast<PlatformLoader>(scheme_router);
  auto transform_stage = TransformLoaderIOS::Make<TransformLoaderIOS>(context);

  pipeline_ = platform_stage_ | transform_stage;

  auto factory = [context.ability getService:@protocol(AnimaXResourceFactoryService)];
  if (factory) {
    SetupResourceFactory(factory);
  }
}

void ResourcePipelineIOS::Load(ResourceRequest request, CallbackType callback) {
  pipeline_->Load(std::move(request), std::move(callback));
}

void ResourcePipelineIOS::RegisterAnimaXLoaderProtocolForScheme(UriInfo::Scheme scheme,
                                                                id<AnimaXLoaderProtocol> protocol) {
  auto scheme_router = std::static_pointer_cast<SchemeRouterLoader>(platform_stage_);
  auto new_loader = PlatformLoader::Make<LoaderWrapperIOS>(protocol);
  scheme_router->RegisterLoaderForScheme(scheme, new_loader);
}

void ResourcePipelineIOS::SetupResourceFactory(id<AnimaXResourceFactoryService> factory) {
  auto loaders = [factory createAnimaXLoaders];
  for (id<AnimaXLoaderProtocol> loader in loaders) {
    RegisterAnimaXLoaderProtocolForScheme(
        static_cast<lynx::animax::UriInfo::Scheme>([loader getScheme]), loader);
  }
}

}  // namespace animax
}  // namespace lynx
