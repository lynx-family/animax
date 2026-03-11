// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_RESOURCE_PIPELINE_IOS_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_RESOURCE_PIPELINE_IOS_H_

#import <AnimaX/AnimaXLoaderProtocol.h>
#import <Foundation/Foundation.h>

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/resource/resource_loader/ios/transform_task_ios.h"

@class AnimaXFileLoader;
@class AnimaXContext;
@protocol AnimaXResourceFactoryService;

namespace lynx {
namespace animax {

/**
 * @class ResourcePipelineIOS
 * @brief Manages resource loading operations on iOS, serving as a pipeline that
 * integrates loading and transformation stages.
 *
 *
 * @details
 * - The class comes with pre-configured loaders for `UriInfo::Scheme::kDataURL`
 * and `UriInfo::Scheme::kFile` schemes.
 *
 */
class ResourcePipelineIOS : public Loader<ResourceRequest, ResourceResponse> {
 public:
  static std::shared_ptr<ResourcePipelineIOS> MakePipeline(AnimaXContext* context = nullptr);
  ResourcePipelineIOS(AnimaXContext* context = nullptr);
  ~ResourcePipelineIOS() override = default;

  /**
   * Initiates asynchronous loading of resources based on the provided request
   * details. This method ensures that the provided callback is executed on the
   * main thread after the resource is loaded or if an error happens.
   *
   * @param request The resource request detailing the specifics of what is to
   * be loaded.
   * @param callback A function to be called once the resource loading is
   * complete, ensuring execution on the main thread.
   */
  void Load(ResourceRequest request, CallbackType callback) override;

  /**
   * Registers a protocol handler for a specific URI scheme, enabling the
   * pipeline to handle different types of resource requests. It is essential to
   * note that this function is **not thread-safe** and should be managed
   * appropriately to avoid concurrency issues.
   *
   * @param scheme The URI scheme for which a new loader protocol will be
   * registered.
   * @param protocol The protocol handler that will manage loading operations
   * for the specified URI scheme.
   */
  void RegisterAnimaXLoaderProtocolForScheme(UriInfo::Scheme scheme,
                                             id<AnimaXLoaderProtocol> protocol);

  AnimaXFileLoader* GetFileLoader() const { return file_loader_; }

 private:
  Loader<ResourceRequest, TransformRequestIOS>::Ptr platform_stage_{};
  Loader<ResourceRequest, ResourceResponse>::Ptr pipeline_{};
  AnimaXFileLoader* file_loader_;
  void SetupResourceFactory(id<AnimaXResourceFactoryService> factory);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_RESOURCE_PIPELINE_IOS_H_
