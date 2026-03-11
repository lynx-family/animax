// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <AnimaX/AnimaXPlayer.h>

NS_ASSUME_NONNULL_BEGIN
@protocol AnimaXViewProtocol <AnimaXCompositionListenerProtocol>

/// Whether enable tap layer event.
///   If false, the native touchesBegan will not handle touch event.
@property(assign, nonatomic) BOOL enableNativeTapLayerEvent;
/// Whether to allow automatic surface initialization when bounds are not ready.
///   If false, the surface will not be ensured to be created after the composition is ready.
@property(assign, nonatomic) BOOL ignoreAttachStatus;

@property(nonatomic, nonnull, readonly) id<AnimaXPlayerProtocol> player;

// Handle the touch event, send tap to element if enabled.
- (void)handleTouch:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;

@end

NS_ASSUME_NONNULL_END
