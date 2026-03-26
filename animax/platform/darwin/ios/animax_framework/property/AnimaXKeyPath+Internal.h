// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXKeyPath.h>
#include <memory>
#include <string>
#include <vector>

// Forward declaration for native AnimaXKeyPath
namespace lynx {
namespace animax {
class AnimaXKeyPath;
}
}  // namespace lynx

@interface AnimaXKeyPath (Internal)

/**
 * Converts OC AnimaXKeyPath to native C++ AnimaXKeyPath
 * @return Native AnimaXKeyPath or nullptr if conversion not possible
 */
- (std::unique_ptr<lynx::animax::AnimaXKeyPath>)toNative;

/**
 * Converts OC AnimaXKeyPath to std::vector<std::string>
 * @return Vector of strings representing the key path
 */
- (std::vector<std::string>)toStringVector;

/**
 * Creates a KeyPath from a native C++ AnimaXKeyPath.
 * This is intended for internal use.
 * @param nativeKeyPath The native C++ AnimaXKeyPath object
 */
- (instancetype)initWithNativeKeyPath:(const lynx::animax::AnimaXKeyPath &)nativeKeyPath;

@end
