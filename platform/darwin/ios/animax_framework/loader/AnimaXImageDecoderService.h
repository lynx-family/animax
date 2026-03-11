// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXLoaderProtocol.h>
#import <AnimaX/AnimaXService.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Protocol defining a service for image decoder.
 */
@protocol AnimaXImageDecoderService <AnimaXService>

/**
 * Decodes image data into UIImage instance.
 *
 * @param data raw image data to be decoded
 * @return decoded UIImage instance, or nil if decoding fails
 */
- (UIImage *)loadImageData:(NSData *)data;

@end

NS_ASSUME_NONNULL_END
