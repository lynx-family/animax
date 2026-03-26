// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import "LottieAdapterFactory.h"
#import "LottieAdapterOption.h"

@implementation LottieAdapterFactory

static NSMutableDictionary<NSNumber *, id<LottieAdapterCreator>> *sCreatorMap = nil;
static LottieAdapterOption *sDefaultOption = nil;

+ (void)initialize {
  if (self == [LottieAdapterFactory class]) {
    sCreatorMap = [NSMutableDictionary dictionary];
    sDefaultOption = [[LottieAdapterOption alloc] init];
  }
}

#pragma mark - Global Configuration

+ (AnimationType)getDefaultType {
  @synchronized(self) {
    return [self getGlobalOption].type;
  }
}

+ (void)setDefaultType:(AnimationType)type {
  @synchronized(self) {
    if (!sDefaultOption) {
      sDefaultOption = [[LottieAdapterOption alloc] init];
    }
    sDefaultOption.type = type;
  }
}

+ (void)setGlobalOption:(nonnull LottieAdapterOption *)option {
  @synchronized(self) {
    sDefaultOption = [option copy];
  }
}

+ (nonnull LottieAdapterOption *)getGlobalOption {
  @synchronized(self) {
    if (!sDefaultOption) {
      sDefaultOption = [[LottieAdapterOption alloc] init];
    }
    return [sDefaultOption copy];
  }
}

+ (void)registerCreator:(id<LottieAdapterCreator>)creator forType:(AnimationType)type {
  if (creator) {
    @synchronized(self) {
      sCreatorMap[@(type)] = creator;
    }
  }
}

+ (void)unregisterCreatorForType:(AnimationType)type {
  @synchronized(self) {
    [sCreatorMap removeObjectForKey:@(type)];
  }
}

#pragma mark - Private Methods

+ (id<LottieAdapterCreator>)creatorForType:(AnimationType)type {
  @synchronized(self) {
    return sCreatorMap[@(type)];
  }
}

#pragma mark - Factory Methods

+ (id<LottieAdapterProtocol>)createAdapterWithRequest:(LottieAdapterRequest *)request {
  return [[self creatorForType:request.options.type] createAdapterWithRequest:request];
}

@end
