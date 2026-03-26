// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol AnimaXAnimationListener <NSObject>

@optional
/**
 * @param params @{
 *   @"animationID": NSString,
 *   @"current":     NSNumber,
 *   @"total":       NSNumber,
 *   @"loopIndex":   NSNumber,
 * }
 */
- (void)onCompletion:(NSDictionary *)params;
/**
 * @param params @{
 *   @"animationID": NSString,
 *   @"current":     NSNumber,
 *   @"total":       NSNumber,
 *   @"loopIndex":   NSNumber,
 * }
 */
- (void)onStart:(NSDictionary *)params;
/**
 * @param params @{
 *   @"animationID": NSString,
 *   @"current":     NSNumber,
 *   @"total":       NSNumber,
 *   @"loopIndex":   NSNumber,
 * }
 */
- (void)onRepeat:(NSDictionary *)params;
/**
 * It won't be notified currently.
 */
- (void)onCancel:(NSDictionary *)params;
/**
 * @param params @{
 *   @"animationID": NSString,
 *   @"current":     NSNumber,
 *   @"total":       NSNumber,
 *   @"loopIndex":   NSNumber,
 *   @"elementID":   NSString,
 * }
 */
- (void)onReady:(NSDictionary *)params;
/**
 * @param params @{
 *   @"animationID": NSString,
 *   @"current":     NSNumber,
 *   @"total":       NSNumber,
 *   @"loopIndex":   NSNumber,
 * }
 */
- (void)onUpdate:(NSDictionary *)params;
/**
 * @param params @{
 *   @"msg":  NSString,
 *   @"code": NSNumber,
 * }
 */
- (void)onError:(NSDictionary *)params;
/**
 * @param params @{
 *   @"msg":  NSString,
 *   @"code": NSNumber,
 * }
 */
- (void)onWarning:(NSDictionary *)params;
/**
 * @param params @{
 *   @"animationID":   NSString,
 *   @"current":       NSNumber,
 *   @"total":         NSNumber,
 *   @"loopIndex":     NSNumber,
 *   @"max_drop_rate": NSNumber,
 *   @"fps":           NSNumber,
 * }
 */
- (void)onFps:(NSDictionary *)params;

/**
 * @param params @{
 *   @"animationID":   NSString,
 *   @"current":       NSNumber,
 *   @"total":         NSNumber,
 *   @"loopIndex":     NSNumber,
 *   @"layerList": Array
 * }
 */
- (void)onTapLayers:(NSDictionary *)params;

/**
 * @param params @{
 *   @"animationID": NSString,
 *   @"current":     NSNumber,
 *   @"total":       NSNumber,
 *   @"loopIndex":   NSNumber,
 * }
 */
- (void)onFirstFrame:(NSDictionary *)params;

/**
 * @param params @{
 *   @"animationID": NSString,
 *   @"current":     NSNumber,
 *   @"total":       NSNumber,
 *   @"loopIndex":   NSNumber,
 * }
 */
- (void)onCompositionReady:(NSDictionary *)params;

@end

NS_ASSUME_NONNULL_END
