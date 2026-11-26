// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXPlayer.h>

#include "include/player/animax_surface.h"

NS_ASSUME_NONNULL_BEGIN

@interface AnimaXPlayer (Internal)

// A nonempty factory runs on the render thread.
- (void)adoptAnimaXSurfaceFactory:(lynx::animax::SurfaceCreationFactory)factory;

@end

NS_ASSUME_NONNULL_END
