// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "AnimaXLayer.h"

@implementation AnimaXLayer
/**
 * Override the setOpacity method to intercept opacity changes on the layer.
 *
 * This approach solves a core visibility tracking issue:
 * 1. When UIView.setAlpha is called, it automatically calls layer.setOpacity internally
 * 2. However, when layer.setOpacity is called directly, UIView.setAlpha is NOT called
 *
 * By intercepting at the CALayer level, we catch ALL opacity changes regardless of source:
 * - Changes initiated through UIView.alpha property
 * - Direct changes to layer.opacity
 * - Changes from Core Animation
 *
 * This creates a single source of truth for visibility changes due to opacity,
 * eliminating the need to override UIView's setAlpha method.
 *
 * @param opacity The new opacity value to be set (0.0 - 1.0)
 */
- (void)setOpacity:(float)opacity {
  float oldOpacity = self.opacity;
  [super setOpacity:opacity];
  [(id<AnimaXLayerProtocol>)self.delegate handleAlphaChange:oldOpacity newAlpha:opacity];
}

@end
