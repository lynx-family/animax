// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXValueCallback.h>
#import <AnimaX/LayerPropertyType.h>
#include <functional>
#include <memory>

namespace lynx {
namespace animax {
class AnimaXValueCallback;
class Value;
class AnimaXFrameInfo;
enum class LayerPropertyType : uint16_t;
}  // namespace animax
}  // namespace lynx

@interface AnimaXValueCallback (Internal)

/**
 * Creates a native C++ value callback from OC value callback
 * @param iosCallback OC value callback instance
 * @param propertyType Layer property type for the callback
 * @return Native value callback or nullptr if creation failed
 */
+ (std::shared_ptr<lynx::animax::AnimaXValueCallback>)
    createNativeCallback:(AnimaXValueCallback *)iosCallback
            propertyType:(LayerPropertyType)propertyType;

/**
 * Called by native code to get the value for a specific frame.
 * This method creates an AnimaXFrameInfo object and calls getValue:.
 *
 * @param startFrame The start frame of the current keyframe segment
 * @param endFrame The end frame of the current keyframe segment
 * @param startValue The starting value for this segment
 * @param endValue The ending value for this segment
 * @param linearProgress Linear progress from 0 to 1 within this segment
 * @param interpolatedProgress Interpolated progress (with easing applied)
 * @param overallProgress Overall progress of the entire animation from 0 to 1
 * @return The custom value to use, or nil to use the original value
 */
- (nullable id)getValueInternalWithStartFrame:(float)startFrame
                                     endFrame:(float)endFrame
                                   startValue:(AnimaXValueParam *)startValue
                                     endValue:(AnimaXValueParam *)endValue
                               linearProgress:(float)linearProgress
                         interpolatedProgress:(float)interpolatedProgress
                              overallProgress:(float)overallProgress;

@end
