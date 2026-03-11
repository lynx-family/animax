// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXService.h>
#import <AnimaX/AnimaXServiceScope.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * AnimaXServiceRegistry is used for instance-level service management,
 * Each Ability instance can hold a dedicated registry for services relevant only to that instance.
 */
@interface AnimaXServiceRegistry : NSObject

/**
 * The scope associated with this registry instance.
 */
@property(nonatomic, strong, readonly) AnimaXServiceScope *scope;

/**
 * Designated initializer to create a registry for a specified scope.
 *
 * @param scope The scope object
 */
- (instancetype)initWithScope:(AnimaXServiceScope *)scope NS_DESIGNATED_INITIALIZER;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Retrieves a service implementation for the specified protocol.
 *
 * @param proto The protocol representing the service
 * @return The service implementation if found, nil otherwise
 */
- (nullable id<AnimaXService>)getService:(Protocol *)proto;

/**
 * Registers a service implementation for the given protocol.
 * If the protocol is already registered, the old implementation will be overwritten.
 *
 * @param proto The protocol
 * @param impl  The service implementation
 */
- (void)registerService:(Protocol *)proto withImpl:(id<AnimaXService>)impl;

/**
 * Unregisters the service for the specified protocol.
 *
 * @param proto The protocol to unregister
 */
- (void)unregisterService:(Protocol *)proto;

/**
 * Releases (unregisters) all services from this registry.
 */
- (void)releaseAll;

@end

NS_ASSUME_NONNULL_END
