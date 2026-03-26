// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "AnimaXResourceLoaderHolder.h"
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXServiceCenter.h>
#import <AnimaX/AnimaXServiceScope.h>
#import "AnimaXContext+Internal.h"
#import "AnimaXFileLoader.h"
#import "AnimaXResourceLoaderHolder+Internal.h"
#include "include/player/animax_player_builder.h"
#include "src/resource/composition_loader/animax_composition_factory.h"
#include "src/resource/resource_loader/ios/resource_pipeline_ios.h"
#include "src/resource/resource_loader/ios/unzip_loader_ios.h"

@interface AnimaXResourceLoaderHolder ()
- (instancetype)initPrivate;
@end

@implementation AnimaXResourceLoaderHolder {
  std::shared_ptr<lynx::animax::ResourcePipelineIOS> _resourcePipeline;
  std::shared_ptr<lynx::animax::UnzipLoaderIOS> _unzipLoader;
}

- (instancetype)initPrivate:(AnimaXContext *)context {
  self = [super init];
  if (self) {
    _resourcePipeline = lynx::animax::ResourcePipelineIOS::MakePipeline(context);
    _unzipLoader =
        std::shared_ptr<lynx::animax::UnzipLoaderIOS>{new lynx::animax::UnzipLoaderIOS{}};
  }
  return self;
}

+ (AnimaXResourceLoaderHolder *)loaderForPlayer:(void *)builderPtr
                                        context:(AnimaXContext *)context {
  if (!builderPtr) {
    return nil;
  }

  auto builder = static_cast<lynx::animax::AnimaXPlayerBuilder *>(builderPtr);
  if (!builder) {
    return nil;
  }

  AnimaXResourceLoaderHolder *loader = [[AnimaXResourceLoaderHolder alloc] initPrivate:context];
  builder->SetResourceLoader(loader->_resourcePipeline).SetUnzipLoader(loader->_unzipLoader);
  return loader;
}

+ (AnimaXResourceLoaderHolder *)loaderForCompositionFactory:(AnimaXContext *)context {
  AnimaXServiceScope *scope = context.ability.getServiceScope;
  AnimaXResourceLoaderHolder *loader = [[AnimaXResourceLoaderHolder alloc] initPrivate:context];
  lynx::animax::AnimaXCompositionFactory::Instance().CreateLoader(
      scope.description.UTF8String, loader->_resourcePipeline, loader->_unzipLoader);
  return loader;
}

- (void)dealloc {
  _resourcePipeline.reset();
  _unzipLoader.reset();
}

- (void)setAssetBundle:(NSBundle *)bundle {
  AnimaXFileLoader *fileLoader = _resourcePipeline->GetFileLoader();
  [fileLoader setAssetBundle:bundle];
}
@end
