// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXAnimationListener.h>
#import <AnimaX/AnimaXMonitorAbilityDelegate.h>
#import <AnimaX/AnimaXMonitorService.h>
#import <AnimaX/AnimaXService.h>
#import <AnimaX/AnimaXServiceCenter.h>
#import <AnimaX/AnimaXServiceRegistry.h>
#import <AnimaX/AnimaXServiceScope.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * BaseAnimaXAbility serves as the foundation class for animation abilities.
 * It provides service management and event handling capabilities.
 *
 * Key responsibilities:
 * - Managing service registry for dependency injection
 * - Handling animation event listeners
 * - Coordinating with monitoring delegate
 */
@interface BaseAnimaXAbility : NSObject

#pragma mark - Initialization

/**
 * Designated initializer.
 * Each ability determines its own service scope via getServiceScope.
 *
 * @return An initialized instance of BaseAnimaXAbility
 */
- (instancetype)init NS_DESIGNATED_INITIALIZER;
+ (instancetype)new NS_UNAVAILABLE;

#pragma mark - Service Management

/**
 * Retrieves a service implementation for the specified protocol.
 * First attempts to find in local registry, then falls back to global service center.
 *
 * @param proto The protocol identifying the desired service
 * @return The service implementation or nil if not found
 */
- (nullable id<AnimaXService>)getService:(Protocol *)proto;

/**
 * Registers a service implementation in the local registry.
 *
 * @param proto The protocol identifying the service
 * @param impl The service implementation
 */
- (void)registerService:(Protocol *)proto withImpl:(id<AnimaXService>)impl;

/**
 * Removes a service registration from the local registry.
 *
 * @param proto The protocol identifying the service to remove
 */
- (void)unregisterService:(Protocol *)proto;

#pragma mark - Event Handling

/**
 * Adds an animation event listener.
 * Listeners are stored as weak references.
 *
 * @param listener The listener to add
 */
- (void)addAnimationListener:(id<AnimaXAnimationListener>)listener;

/**
 * Removes an animation event listener.
 *
 * @param listener The listener to remove
 */
- (void)removeAnimationListener:(id<AnimaXAnimationListener>)listener;

/**
 * Returns a copy of all currently registered listeners.
 *
 * @return A hash table containing all animation listeners
 */
- (NSHashTable<id<AnimaXAnimationListener>> *)allListeners;

/**
 * Processes animation events and distributes them to registered listeners.
 *
 * @param event The event pointer
 * @param params Additional parameters for the event
 */
- (void)handleEvent:(void *)event params:(NSDictionary *)params;

#pragma mark - Monitor Delegate

/**
 * Returns the monitor delegate associated with this ability.
 *
 * @return The monitor delegate instance
 */
- (AnimaXMonitorAbilityDelegate *)getMonitorDelegate;

#pragma mark - Service Scope

/**
 * Returns the service scope for this ability.
 * Default implementation returns the default service scope.
 *
 * @return The service scope for this ability
 */
- (AnimaXServiceScope *)getServiceScope;

@end

NS_ASSUME_NONNULL_END
