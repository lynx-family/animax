// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXPlayerProtocol.h>

#ifdef __cplusplus
#include <memory>

namespace lynx {
namespace animax {
class AnimaXPlayer;
}
}  // namespace lynx
#endif

@protocol AnimaXCompositionListenerProtocol <NSObject>

- (void)onCompositionReady;

@end

@class UIView;

@interface AnimaXPlayer : NSObject <AnimaXPlayerProtocol>

@property(nonatomic, weak) id<AnimaXCompositionListenerProtocol> compositionListener;
@property(nonatomic, weak, nullable) UIView *lifecycleView;
@property(nonatomic, assign, readonly) CGFloat scale;

- (instancetype)initWithContext:(AnimaXContext *)context;
- (instancetype)initWithContext:(AnimaXContext *)context scale:(CGFloat)scale;

#ifdef __cplusplus
// Platform integrations use the existing player rather than creating another
// receiver or retaining a separate native lifetime.
- (std::shared_ptr<lynx::animax::AnimaXPlayer>)getPlayerShared;
#endif

@end
