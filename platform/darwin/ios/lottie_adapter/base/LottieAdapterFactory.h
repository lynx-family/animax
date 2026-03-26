// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import "LottieAdapterCreationProtocol.h"
#import "LottieAdapterCreator.h"
#import "LottieAdapterOption.h"
#import "LottieAdapterProtocol.h"

NS_ASSUME_NONNULL_BEGIN

@interface LottieAdapterFactory : NSObject

/// Deprecated: use `setGlobalOption:` instead.
+ (void)setDefaultType:(AnimationType)type;
/// Deprecated: use `getGlobalOption` instead.
+ (AnimationType)getDefaultType;
+ (void)setGlobalOption:(nonnull LottieAdapterOption *)option;
+ (nonnull LottieAdapterOption *)getGlobalOption;
+ (void)registerCreator:(id<LottieAdapterCreator>)creator forType:(AnimationType)type;
+ (void)unregisterCreatorForType:(AnimationType)type;

+ (id<LottieAdapterProtocol>)createAdapterWithRequest:(nonnull LottieAdapterRequest *)request;

@end

NS_ASSUME_NONNULL_END
