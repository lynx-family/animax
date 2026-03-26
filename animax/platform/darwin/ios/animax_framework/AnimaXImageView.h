// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXPlayerProtocol.h>
#import <AnimaX/AnimaXViewProtocol.h>

NS_ASSUME_NONNULL_BEGIN

@interface AnimaXImageView : UIView <AnimaXPlayerProtocol, AnimaXViewProtocol>

// redeclare it to generate getter/setter.
@property(assign, nonatomic) BOOL enableNativeTapLayerEvent;
@property(assign, nonatomic) BOOL ignoreAttachStatus;
@property(nonatomic, nonnull, readonly) id<AnimaXPlayerProtocol> player;

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithFrame:(CGRect)frame NS_UNAVAILABLE;
- (instancetype)initWithCoder:(NSCoder *)coder NS_UNAVAILABLE;

- (instancetype)initWithContext:(AnimaXContext *)context NS_DESIGNATED_INITIALIZER;
- (instancetype)initWithPlayer:(AnimaXPlayer *)player NS_DESIGNATED_INITIALIZER;

@end
NS_ASSUME_NONNULL_END
