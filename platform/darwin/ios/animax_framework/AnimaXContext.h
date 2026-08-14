// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/BaseAnimaXAbility.h>

NS_ASSUME_NONNULL_BEGIN

@interface AnimaXContext : NSObject

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

@property(nonatomic, strong, nullable) BaseAnimaXAbility *ability;
@property(nonatomic, assign) BOOL enableMultiThreadAccelerate;
@property(nonatomic, assign) BOOL disablePlaybackOnAssetLoadFailure;
@property(nonatomic, assign) BOOL enableDownsampleVideo;
@property(nonatomic, assign) CGFloat scale;

+ (instancetype)contextWithDefaultAbility;  // For client usage
- (instancetype)initWithAbility:(BaseAnimaXAbility *_Nullable)ability NS_DESIGNATED_INITIALIZER;

@end

NS_ASSUME_NONNULL_END
