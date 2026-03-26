// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "LottieAdapterOption.h"
#import "LottieAdapterFactory.h"

@interface LottieAdapterRequest ()
@property(nonatomic, assign, readwrite) LottieAdapterAnimationSourceType sourceType;
@property(nonatomic, copy, readwrite, nullable) NSString *name;
@property(nonatomic, strong, readwrite, nullable) NSBundle *bundle;
@property(nonatomic, strong, readwrite, nullable) NSDictionary *json;
@property(nonatomic, copy, readwrite, nullable) NSString *filePath;
@property(nonatomic, strong, readwrite, nullable) NSURL *url;
@property(nonatomic, copy, readwrite, nullable) LottieAdapterOption *options;
@end

@implementation LottieAdapterOption
- (instancetype)init {
  self = [super init];
  if (self) {
    self.type = AnimationTypeAnimaX;
    self.useImageMode = NO;
    self.multiThreadAccelerate = NO;
  }
  return self;
}

- (nonnull id)copyWithZone:(nullable NSZone *)zone {
  LottieAdapterOption *opt = [[[self class] allocWithZone:zone] init];
  opt.type = self.type;
  opt.useImageMode = self.useImageMode;
  opt.multiThreadAccelerate = self.multiThreadAccelerate;
  return opt;
}

@end

@implementation LottieAdapterRequest

- (instancetype)init {
  self = [super init];
  if (self) {
    self.sourceType = LottieAdapterAnimationNone;
    self.bundle = [NSBundle mainBundle];
    self.options = [LottieAdapterFactory getGlobalOption];
  }
  return self;
}

+ (instancetype)animationWithOptions:(nullable LottieAdapterOption *)options {
  LottieAdapterRequest *request = [[self alloc] init];
  if (options) {
    request.options = options;
  }
  return request;
}

+ (instancetype)animationNamed:(nonnull NSString *)name
                      inBundle:(nullable NSBundle *)bundle
                       options:(nullable LottieAdapterOption *)options {
  LottieAdapterRequest *request = [[self alloc] init];
  request.name = name;
  request.sourceType = LottieAdapterAnimationNamed;
  if (bundle) {
    request.bundle = bundle;
  }
  if (options) {
    request.options = options;
  }
  return request;
}

+ (instancetype)animationFromJSON:(nonnull NSDictionary *)json
                         inBundle:(nullable NSBundle *)bundle
                          options:(nullable LottieAdapterOption *)options {
  LottieAdapterRequest *request = [[self alloc] init];
  request.json = json;
  request.sourceType = LottieAdapterAnimationJSON;
  if (bundle) {
    request.bundle = bundle;
  }
  if (options) {
    request.options = options;
  }
  return request;
}
+ (instancetype)animationWithFilePath:(nonnull NSString *)filePath
                              options:(nullable LottieAdapterOption *)options {
  LottieAdapterRequest *request = [[self alloc] init];
  request.filePath = filePath;
  request.sourceType = LottieAdapterAnimationFilePath;
  if (options) {
    request.options = options;
  }
  return request;
}
+ (instancetype)animationWithURL:(nonnull NSURL *)url
                         options:(nullable LottieAdapterOption *)options {
  LottieAdapterRequest *request = [[self alloc] init];
  request.url = url;
  request.sourceType = LottieAdapterAnimationURL;
  if (options) {
    request.options = options;
  }
  return request;
}

@end
