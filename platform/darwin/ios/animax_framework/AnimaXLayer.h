// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

NS_ASSUME_NONNULL_BEGIN
@protocol AnimaXLayerProtocol <NSObject>

/**
 * Handle changes in alpha value for the view.
 * This method is called by AnimaXLayer when its opacity changes.
 * @param oldAlpha The previous alpha value
 * @param newAlpha The new alpha value
 */
- (void)handleAlphaChange:(CGFloat)oldAlpha newAlpha:(CGFloat)newAlpha;

@end

@interface AnimaXLayer : CALayer
@end

NS_ASSUME_NONNULL_END
