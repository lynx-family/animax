// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_LOADER_WRAPPER_IOS_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_LOADER_WRAPPER_IOS_H_

#import <AnimaX/AnimaXLoaderProtocol.h>

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/resource/resource_loader/ios/transform_task_ios.h"

namespace lynx {
namespace animax {

class LoaderWrapperIOS : public Loader<ResourceRequest, TransformRequestIOS> {
 public:
  LoaderWrapperIOS() = default;

  ~LoaderWrapperIOS() override = default;

  LoaderWrapperIOS(id<AnimaXLoaderProtocol> loader) : ios_loader_(loader) {}

  void Load(ResourceRequest, CallbackType callback) override;

 private:
  id<AnimaXLoaderProtocol> ios_loader_ = nil;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_LOADER_WRAPPER_IOS_H_
