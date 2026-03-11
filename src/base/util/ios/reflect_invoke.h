// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_IOS_REFLECT_INVOKE_H_
#define ANIMAX_SRC_BASE_UTIL_IOS_REFLECT_INVOKE_H_

#include <string>

namespace lynx {
namespace animax {

/**
 * @brief Executes a class method on a specified Objective-C class exactly once
 * in a thread-safe manner.
 *
 * This function uses Grand Central Dispatch's `dispatch_once` to ensure that
 * the specified method on the given class is called only once during the
 * lifetime of the application, even in a multithreaded environment. It
 * dynamically looks up the class and method by name using the Objective-C
 * runtime.
 *
 * @param class_name The name of the Objective-C class as a UTF-8 encoded
 * std::string.
 * @param function_name The name of the class method (selector) to invoke, as a
 * UTF-8 encoded std::string.
 *
 * @note This function must be implemented in an Objective-C++ (.mm) file
 * because it uses Objective-C runtime features.
 * @note The specified method should be a class method (+), not an instance
 * method (-).
 * @note If the class or method does not exist, the function does nothing.
 */
extern void ReflectInvoke(const std::string& class_name,
                          const std::string& function_name);

}  // namespace animax
}  // namespace lynx
#endif  // ANIMAX_SRC_BASE_UTIL_IOS_REFLECT_INVOKE_H_
