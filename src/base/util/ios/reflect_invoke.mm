// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <string>

#import <Foundation/Foundation.h>

namespace lynx {
namespace animax {
void ReflectInvoke(const std::string& class_name, const std::string& function_name) {
  NSString* ns_class_name = [NSString stringWithUTF8String:class_name.c_str()];
  Class cur_class = NSClassFromString(ns_class_name);
  if (cur_class) {
    NSString* ns_function_name = [NSString stringWithUTF8String:function_name.c_str()];
    SEL selector = NSSelectorFromString(ns_function_name);
    if ([cur_class respondsToSelector:selector]) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
      [cur_class performSelector:selector];
#pragma clang diagnostic pop
    }
  }
}
}  // namespace animax
}  // namespace lynx
