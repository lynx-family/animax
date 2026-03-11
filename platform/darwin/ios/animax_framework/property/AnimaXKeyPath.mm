// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXKeyPath.h>
#include <string>
#include <vector>
#import "AnimaXKeyPath+Internal.h"
#include "include/property/animax_key_path.h"
#include "src/base/log/log.h"

@implementation AnimaXKeyPath

- (instancetype)initWithKeys:(NSArray<NSString *> *)keys {
  self = [super init];
  if (self) {
    NSMutableArray<NSString *> *validKeys = [NSMutableArray array];
    for (NSString *key in keys) {
      if (key == nil || key.length == 0) {
        ANIMAX_LOGI("AnimaXKeyPath received a nil or empty key, which will be ignored.");
        continue;
      }
      [validKeys addObject:key];
    }
    _keys = [validKeys copy];
  }
  return self;
}

- (NSString *)description {
  return [NSString stringWithFormat:@"AnimaXKeyPath{keys=%@}", _keys];
}

@end

@implementation AnimaXKeyPath (Internal)

- (std::unique_ptr<lynx::animax::AnimaXKeyPath>)toNative {
  std::vector<std::string> keys = [self toStringVector];
  return std::unique_ptr<lynx::animax::AnimaXKeyPath>(new lynx::animax::AnimaXKeyPath(keys));
}

- (std::vector<std::string>)toStringVector {
  std::vector<std::string> keys;

  NSArray<NSString *> *keysArray = self.keys;
  keys.reserve(keysArray.count);

  for (NSString *key in keysArray) {
    if (key != nil) {
      keys.push_back([key UTF8String]);
    }
  }

  return keys;
}

- (instancetype)initWithNativeKeyPath:(const lynx::animax::AnimaXKeyPath &)nativeKeyPath {
  NSMutableArray<NSString *> *keys = [NSMutableArray array];
  for (const auto &key : nativeKeyPath.GetKeys()) {
    [keys addObject:[NSString stringWithUTF8String:key.c_str()]];
  }
  // Call the designated initializer of the main class
  return [self initWithKeys:keys];
}

@end
