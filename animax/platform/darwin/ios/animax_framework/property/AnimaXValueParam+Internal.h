// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXValueParam.h>
#include <memory>
#include <string>

namespace lynx {
namespace animax {
class AnimaXValueParam;
class Value;
}  // namespace animax
}  // namespace lynx

@interface AnimaXValueParam (Internal)

/**
 * Converts OC AnimaXValueParam to native C++ AnimaXValueParam
 * @return Native AnimaXValueParam or nullptr if conversion not possible
 */
- (std::unique_ptr<lynx::animax::AnimaXValueParam>)toNative;

/**
 * Creates OC AnimaXValueParam from native Value
 * @param value Native Value to convert
 * @return New AnimaXValueParam instance or nil if conversion not possible
 */
+ (nullable instancetype)valueParamWithNativeValue:(const lynx::animax::Value*)value;

/**
 * Creates OC AnimaXValueParam from native AnimaXValueParam
 * @param valueParam Native AnimaXValueParam to convert
 * @return New AnimaXValueParam instance or nil if conversion not possible
 */
+ (nullable instancetype)valueParamWithNative:(const lynx::animax::AnimaXValueParam*)valueParam;

@end
