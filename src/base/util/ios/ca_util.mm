// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/ios/ca_util.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

#pragma mark - CAMetalLayer

CAMetalLayer* CreateCAMetalLayer(CGRect frame, CGFloat scale) {
  CAMetalLayer* layer = [[CAMetalLayer alloc] init];
  [layer setDevice:MTLCreateSystemDefaultDevice()];
  [layer setPixelFormat:MTLPixelFormatBGRA8Unorm];
  [layer setFrame:CGRectMake(0, 0, frame.size.width, frame.size.height)];
  [layer setOpaque:NO];
  [layer setContentsScale:scale];
  return layer;
}

#pragma mark - Common

void ResizeCALayer(CALayer* layer, CGSize size) {
  layer.frame = CGRectMake(layer.bounds.origin.x, layer.bounds.origin.y, size.width, size.height);
}

void ResizeCAMetalLayerDrawable(CAMetalLayer* layer, CGSize size) {
  // Update CAMetalLayer's drawable size to match the new layer size.
  // This step is crucial to ensure that the drawable obtained from "nextDrawable"
  // matches the current size. The size here is measured in pixels (px), because we’ve already
  // unified the unit inside the drawable.
  [(CAMetalLayer*)layer setDrawableSize:size];
}

}  // namespace animax
}  // namespace lynx
