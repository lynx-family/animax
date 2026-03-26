// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import <Lynx/LynxAnimaXImageDecoderServiceImpl.h>
#import <Lynx/LynxService.h>
#import <Lynx/LynxServiceImageProtocol.h>

@implementation LynxAnimaXImageDecoderServiceImpl

- (UIImage *)loadImageData:(NSData *)data {
  return [LynxService(LynxServiceImageProtocol) decodeImage:data];
}

@end
