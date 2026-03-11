// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaX.h>
#include "src/base/util/animax_native.h"

#if __has_include("BDALog/BDAgileLog.h")
#include "BDALog/BDAgileLog.h"
#define BD_ALOG_ENABLED 1
#endif

#ifdef BD_ALOG_ENABLED
[[maybe_unused]] void BDAlogWrite(unsigned int level, const char *tag, const char *format) {
  if (format == nullptr) {
    return;
  }
  BDLoggerInfo info;

  info.filename = "";
  info.tag = tag;
  info.line = -1;
  info.func_name = "";
  struct timeval tv;
  gettimeofday(&tv, NULL);
  info.timeval = tv;
  info.level = static_cast<kBDALogLevel>(level);

  _alog_write_macro(&info, format);
}
#endif  // BD_ALOG_ENABLED

lynx::animax::ALogFunction GetAlogWriteFuncAddr() {
#ifdef BD_ALOG_ENABLED
  return lynx::animax::ALogFunction(&BDAlogWrite);
#else
  return nullptr;
#endif
}

@implementation AnimaX
+ (instancetype)shareInstance {
  static AnimaX *_instance = nil;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    _instance = [[self alloc] init];
  });

  return _instance;
}

- (void)registerLoggerOnce {
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    lynx::animax::AnimaXNative::Instance().RegisterALogFunction(GetAlogWriteFuncAddr());
  });
}

@end
