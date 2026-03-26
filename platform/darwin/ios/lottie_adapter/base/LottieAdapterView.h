// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <UIKit/UIKit.h>
#import "LottieAdapterCreationProtocol.h"
#import "LottieAdapterProtocol.h"

NS_ASSUME_NONNULL_BEGIN

/// Preferred creation interface for the Adapter.
/// This view wraps the underlying adapter and provides clear, recommended initializers
/// to construct animations with process-wide defaults or per-instance options.
///
/// Example :
///   LottieAdapterOption *option = [[LottieAdapterOption alloc] init];
///   option.type = AnimationTypeAnimaX;
///   option.useImageMode = YES;
///   option.multiThreadAccelerate = YES;
///   [LottieAdapterFactory setGlobalOption:option];
///
///   // Create animation using global options
///   LottieAdapterView *animation1 = [[LottieAdapterView alloc] init];
///
///   option.useImageMode = NO;
///   // Create animation using instance options
///   LottieAdapterView *animation2 = [[LottieAdapterView alloc] initWithOptions:options];
@interface LottieAdapterView : UIView <LottieAdapterProtocol, LottieAdapterCreationProtocol>

- (instancetype)init NS_SWIFT_NAME(init());

- (instancetype)initWithOptions:(nullable LottieAdapterOption *)options
    NS_SWIFT_NAME(init(options:)) NS_DESIGNATED_INITIALIZER;

- (instancetype)initWithName:(NSString *)name
                      bundle:(NSBundle *_Nullable)bundle NS_SWIFT_NAME(init(name:bundle:));

- (instancetype)initWithName:(NSString *)name
                      bundle:(NSBundle *_Nullable)bundle
                     options:(nullable LottieAdapterOption *)options
    NS_SWIFT_NAME(init(name:bundle:options:)) NS_DESIGNATED_INITIALIZER;

- (instancetype)initWithContentsOfURL:(nonnull NSURL *)url
                              options:(nullable LottieAdapterOption *)option
    NS_DESIGNATED_INITIALIZER;

- (instancetype)initWithCoder:(NSCoder *)coder NS_UNAVAILABLE;
- (instancetype)initWithFrame:(CGRect)frame NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
