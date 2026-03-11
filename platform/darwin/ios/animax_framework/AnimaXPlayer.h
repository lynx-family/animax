// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXPlayerProtocol.h>

@protocol AnimaXCompositionListenerProtocol <NSObject>

- (void)onCompositionReady;

@end

@interface AnimaXPlayer : NSObject <AnimaXPlayerProtocol>

@property(nonatomic, weak) id<AnimaXCompositionListenerProtocol> compositionListener;

- (instancetype)initWithContext:(AnimaXContext *)context;
- (instancetype)initWithContext:(AnimaXContext *)context scale:(CGFloat)scale;

@end
