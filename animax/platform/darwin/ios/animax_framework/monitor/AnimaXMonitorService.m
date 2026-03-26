// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXMonitorService.h>
#import <objc/runtime.h>

// URL Related Keys
NSString *const AnimaXMonitorSourceUrl = @"src_url";
NSString *const AnimaXMonitorPageUrl = @"page_url";

// Associated Object Key
void const *AnimaXMonitorUrlHolderKey = &AnimaXMonitorUrlHolderKey;

@implementation AnimaXMonitorUrlHolder

- (instancetype)init {
  self = [super init];
  if (self) {
    _currentUrl = @"Unknown";
  }
  return self;
}

- (void)updateUrl:(NSString *)url {
  self.currentUrl = url ?: self.currentUrl;
}

- (NSString *)getCurrentUrl {
  return self.currentUrl;
}

@end

// Default implementation for URL holder management
@implementation NSObject (AnimaXMonitorServiceDefaultImpl)

- (AnimaXMonitorUrlHolder *)urlHolder {
  AnimaXMonitorUrlHolder *holder = objc_getAssociatedObject(self, AnimaXMonitorUrlHolderKey);
  if (!holder) {
    holder = [[AnimaXMonitorUrlHolder alloc] init];
    objc_setAssociatedObject(self, AnimaXMonitorUrlHolderKey, holder,
                             OBJC_ASSOCIATION_RETAIN_NONATOMIC);
  }
  return holder;
}

@end
