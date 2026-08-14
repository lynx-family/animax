// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXComposition.h>
#import <AnimaX/AnimaXCompositionFactory.h>
#import <AnimaX/AnimaXServiceScope.h>
#import <AnimaX/DefaultAnimaXAbility.h>
#import "AnimaXComposition+Internal.h"
#import "AnimaXResourceLoaderHolder+Internal.h"
#import "AnimaXResourceLoaderHolder.h"
#import "AnimaXThreadSafeDictionary.h"
#include "src/resource/composition_loader/animax_composition_factory.h"

@interface AnimaXCompositionFactory ()
@property(nonatomic, strong)
    AnimaXThreadSafeDictionary<AnimaXServiceScope *, AnimaXResourceLoaderHolder *> *loaderMap;
@end

@implementation AnimaXCompositionFactory

+ (AnimaXCompositionFactory *)shared {
  static AnimaXCompositionFactory *sInstance = nil;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    sInstance = [[AnimaXCompositionFactory alloc] initPrivate];
  });
  return sInstance;
}

- (instancetype)initPrivate {
  self = [super init];
  if (self) {
    _loaderMap = [[AnimaXThreadSafeDictionary alloc] init];
  }
  return self;
}

- (void)ensureLoaderForContext:(AnimaXContext *)context {
  AnimaXServiceScope *scope = context.ability.getServiceScope;
  if (!self.loaderMap[scope]) {
    self.loaderMap[scope] = [AnimaXResourceLoaderHolder loaderForCompositionFactory:context];
  }
}

- (void)handleCompositionModel:(std::shared_ptr<lynx::animax::CompositionModel>)model
                         error:(const std::string &)errorMessage
                    completion:(AnimaXCompositionCompletionBlock)completion {
  if (!completion) {
    return;
  }

  if (!model || !errorMessage.empty()) {
    NSError *error = nil;
    if (!errorMessage.empty()) {
      error = [NSError errorWithDomain:@"AnimaXCompositionErrorDomain"
                                  code:-1
                              userInfo:@{NSLocalizedDescriptionKey : @(errorMessage.c_str())}];
    }
    completion(nil, error);
    return;
  }

  AnimaXComposition *composition = [[AnimaXComposition alloc] initWithSharedModel:&model];
  completion(composition, nil);
}

- (void)loadJson:(NSString *)json
     withContext:(AnimaXContext *)context
      completion:(AnimaXCompositionCompletionBlock)completion {
  [self ensureLoaderForContext:context];
  AnimaXServiceScope *scope = context.ability.getServiceScope;
  __weak typeof(self) weakSelf = self;
  lynx::animax::AnimaXCompositionFactory::Instance().LoadJson(
      scope.description.UTF8String, json.UTF8String, context.scale,
      [weakSelf, completion](std::shared_ptr<lynx::animax::CompositionModel> model,
                             const std::string &error) {
        [weakSelf handleCompositionModel:std::move(model) error:error completion:completion];
      });
}

- (void)loadJson:(NSString *)json completion:(AnimaXCompositionCompletionBlock)completion {
  AnimaXContext *context =
      [[AnimaXContext alloc] initWithAbility:[[DefaultAnimaXAbility alloc] init]];
  [self loadJson:json withContext:context completion:completion];
}

- (void)loadUri:(NSString *)uri
    withContext:(AnimaXContext *)context
     completion:(AnimaXCompositionCompletionBlock)completion {
  [self ensureLoaderForContext:context];
  AnimaXServiceScope *scope = context.ability.getServiceScope;
  __weak typeof(self) weakSelf = self;
  lynx::animax::AnimaXCompositionFactory::Instance().LoadUri(
      scope.description.UTF8String, uri.UTF8String, context.scale,
      [weakSelf, completion](std::shared_ptr<lynx::animax::CompositionModel> model,
                             const std::string &error) {
        [weakSelf handleCompositionModel:std::move(model) error:error completion:completion];
      });
}

- (void)loadUri:(NSString *)uri completion:(AnimaXCompositionCompletionBlock)completion {
  AnimaXContext *context =
      [[AnimaXContext alloc] initWithAbility:[[DefaultAnimaXAbility alloc] init]];
  [self loadUri:uri withContext:context completion:completion];
}

- (void)releaseAll {
  [_loaderMap removeAllObjects];
  lynx::animax::AnimaXCompositionFactory::Instance().Release();
}

@end
