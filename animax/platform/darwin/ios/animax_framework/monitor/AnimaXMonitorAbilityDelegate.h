// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXAnimationListener.h>
#import <AnimaX/AnimaXServiceRegistry.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

#pragma mark - Constants

/**
 * AnimaXMonitorAbilityDelegate handles monitoring and performance tracking for animations.
 * This delegate manages performance metrics, error reporting, and lifecycle events.
 */
@interface AnimaXMonitorAbilityDelegate : NSObject <AnimaXAnimationListener>

#pragma mark - Initialization

/**
 * Initializes the monitor delegate with a service registry.
 *
 * @param serviceRegistry The service registry for managing monitor services
 * @return An initialized instance of AnimaXMonitorAbilityDelegate
 */
- (instancetype)initWithServiceRegistry:(AnimaXServiceRegistry *)serviceRegistry
    NS_DESIGNATED_INITIALIZER;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

#pragma mark - Configuration

/**
 * Sets the native AnimaX player for monitoring.
 *
 * @param player Pointer to the native AnimaX player
 */
- (void)setAnimaXPlayer:(void *)player;

/**
 * Set which display mode the animation is at.
 *
 * @param mode "image" or "surface"
 */
- (void)setDisplayMode:(NSString *)mode;

/**
 * Set the tag for experiment.
 *
 * @param tag tag for experiment.
 */
- (void)setTag:(NSString *)tag;

/**
 * Updates the current URL for monitoring context.
 *
 * @param url The URL to be monitored
 */
- (void)updateUrl:(NSString *)url;

/**
 * Called when the monitored player is being released.
 * Reports final performance metrics and cleans up resources.
 */
- (void)onRelease;

/**
 * Handles play method by reporting onPlay metrics.
 */

- (void)onPlay;

/**
 * Handles resume method by reporting onPlay metrics.
 */
- (void)onResume;

/**
 * Handles playSegment method by reporting onPlay metrics.
 */
- (void)onPlaySegment;

@end

NS_ASSUME_NONNULL_END
