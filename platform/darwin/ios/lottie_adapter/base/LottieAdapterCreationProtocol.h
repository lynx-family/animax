// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <Foundation/Foundation.h>

#import "LottieAdapterOption.h"

NS_ASSUME_NONNULL_BEGIN

@protocol LottieAdapterCreationProtocol <NSObject>
+ (nonnull instancetype)animationNamed:(nonnull NSString *)animationName NS_SWIFT_NAME(init(name:));
+ (nonnull instancetype)animationNamed:(nonnull NSString *)animationName
                              inBundle:(nullable NSBundle *)bundle
    NS_SWIFT_NAME(init(name:bundle:));
+ (nonnull instancetype)animationFromJSON:(nonnull NSDictionary *)animationJSON
    NS_SWIFT_NAME(init(json:));
+ (nonnull instancetype)animationWithFilePath:(nonnull NSString *)filePath
    NS_SWIFT_NAME(init(filePath:));
+ (nonnull instancetype)animationFromJSON:(nonnull NSDictionary *)animationJSON
                                 inBundle:(nullable NSBundle *)bundle
    NS_SWIFT_NAME(init(json:bundle:));
+ (nonnull instancetype)animationNamed:(nonnull NSString *)animationName
                               options:(nullable LottieAdapterOption *)option
    NS_SWIFT_NAME(init(name:options:));
+ (nonnull instancetype)animationNamed:(nonnull NSString *)animationName
                              inBundle:(nullable NSBundle *)bundle
                               options:(nullable LottieAdapterOption *)option
    NS_SWIFT_NAME(init(name:bundle:options:));
+ (nonnull instancetype)animationFromJSON:(nonnull NSDictionary *)animationJSON
                                  options:(nullable LottieAdapterOption *)option
    NS_SWIFT_NAME(init(json:options:));
+ (nonnull instancetype)animationWithFilePath:(nonnull NSString *)filePath
                                      options:(nullable LottieAdapterOption *)option
    NS_SWIFT_NAME(init(filePath:options:));
+ (nonnull instancetype)animationFromJSON:(nonnull NSDictionary *)animationJSON
                                 inBundle:(nullable NSBundle *)bundle
                                  options:(nullable LottieAdapterOption *)option
    NS_SWIFT_NAME(init(json:bundle:options:));
- (nonnull instancetype)initWithContentsOfURL:(nonnull NSURL *)url;
- (nonnull instancetype)initWithContentsOfURL:(nonnull NSURL *)url
                                      options:(nullable LottieAdapterOption *)option;
@end

NS_ASSUME_NONNULL_END
