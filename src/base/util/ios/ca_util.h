// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_IOS_CA_UTIL_H_
#define ANIMAX_SRC_BASE_UTIL_IOS_CA_UTIL_H_

#import <CoreFoundation/CoreFoundation.h>
#import <Metal/Metal.h>
#include <memory>

namespace lynx {
namespace animax {

CAMetalLayer* CreateCAMetalLayer(CGRect frame, CGFloat scale);

// Resizes the CALayer's frame. Must be called from the main thread to ensure thread-safe
// modifications to UI elements.
void ResizeCALayer(CALayer* layer, CGSize size);

// Resizes the CAMetalLayer's drawable. Must be called from the GPU thread to prevent
// inconsistencies with the drawable size updates. Calling from the main thread may lead to
// unsynchronized drawable dimensions on the GPU thread.
void ResizeCAMetalLayerDrawable(CAMetalLayer* layer, CGSize size);

}  // namespace animax
}  // namespace lynx
#endif  // ANIMAX_SRC_BASE_UTIL_IOS_CA_UTIL_H_
