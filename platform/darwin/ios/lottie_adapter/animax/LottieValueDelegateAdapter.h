// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXKeyPath.h>
#import <AnimaX/AnimaXValueCallback.h>
#import <AnimaX/LayerPropertyType.h>
#import <Foundation/Foundation.h>
#import "LottieKeyPath.h"
#import "LottieValueDelegate.h"

@interface LottieValueDelegateAdapter : NSObject

+ (AnimaXValueCallback *)createAnimaXValueCallbackWithLottieDelegate:
                             (id<LottieValueDelegate>)delegate
                                                          forKeyPath:(LottieKeyPath *)keyPath;

+ (LayerPropertyType)getLayerPropertyTypeForKeyPath:(LottieKeyPath *)keyPath;
+ (NSArray<NSString *> *)trimmedKeysForKeyPath:(LottieKeyPath *)keyPath
                                  propertyType:(LayerPropertyType)propertyType;

@end
