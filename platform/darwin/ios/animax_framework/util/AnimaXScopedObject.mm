// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXScopedObject.h>

@interface AnimaXScopedObject ()

@property(nonatomic, assign) AnimaXScopedObjectDeleter deleter;
@property(nonatomic, assign, nullable) const void *rawObject;

@end

@implementation AnimaXScopedObject

- (nullable instancetype)initWithObject:(nullable const void *)object
                                deleter:(AnimaXScopedObjectDeleter)deleter {
  self = [super init];
  if (self) {
    _rawObject = object;
    _deleter = deleter;
  }
  return self;
}

- (void)dealloc {
  if (_deleter != nil && _rawObject != nil) {
    _deleter(_rawObject);
  }
}

@end
