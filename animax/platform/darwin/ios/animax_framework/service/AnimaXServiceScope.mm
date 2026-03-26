// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXServiceScope.h>

@interface AnimaXServiceScope ()
@property(nonatomic, copy, readonly) NSString *value;
@end

@implementation AnimaXServiceScope

static AnimaXServiceScope *_defaultScope;

+ (void)initialize {
  if (self == [AnimaXServiceScope class]) {
    _defaultScope = [[AnimaXServiceScope alloc] initWithValue:@"default"];
  }
}

+ (AnimaXServiceScope *)defaultScope {
  return _defaultScope;
}

+ (instancetype)scopeWithValue:(NSString *)value {
  return [[AnimaXServiceScope alloc] initWithValue:value];
}

- (instancetype)initWithValue:(NSString *)value {
  self = [super init];
  if (self) {
    _value = value.lowercaseString;
  }
  return self;
}

- (BOOL)isEqual:(id)object {
  if (self == object) return YES;
  if (![object isKindOfClass:[AnimaXServiceScope class]]) return NO;

  AnimaXServiceScope *other = (AnimaXServiceScope *)object;
  return [self.value isEqualToString:other.value];
}

- (NSUInteger)hash {
  return self.value.hash;
}

- (NSString *)description {
  return self.value;
}

- (id)copyWithZone:(nullable NSZone *)zone {
  AnimaXServiceScope *copy = [[AnimaXServiceScope allocWithZone:zone] init];
  copy->_value = [_value copyWithZone:zone];
  return copy;
}

@end
