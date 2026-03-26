// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXService.h>
#import <AnimaX/AnimaXServiceScope.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Global service center that manages services across different scopes.
 * This is a singleton providing centralized service management for the entire library.
 *
 * Registration constraints:
 * 1) Each protocol can only have one registered implementation per scope.
 * 2) Attempting to register another implementation for the same protocol will fail,
 *    preserving the original implementation.
 * 3) You must unregister before re-registering a new implementation for the same protocol.
 */
@interface AnimaXServiceCenter : NSObject

/// Access the global singleton instance
+ (instancetype)sharedInstance;

/**
 * Registers a service implementation in the DEFAULT scope.
 *
 * @param proto The protocol specifying the service
 * @param impl  The service implementation
 */
- (void)registerService:(Protocol *)proto withImpl:(id<AnimaXService>)impl;

/**
 * Retrieves a service implementation by protocol from the DEFAULT scope.
 *
 * @param proto The protocol
 * @return The service implementation if found, nil otherwise
 */
- (nullable id<AnimaXService>)getService:(Protocol *)proto;

/**
 * Unregisters a service from the DEFAULT scope by protocol.
 *
 * @param proto The protocol to remove
 */
- (void)unregisterService:(Protocol *)proto;

/**
 * Unregisters all services in the DEFAULT scope.
 */
- (void)unregisterAllServices;

/**
 * Registers a service implementation in the specified scope.
 *
 * @param scope The service scope
 * @param proto The protocol specifying the service
 * @param impl  The service implementation
 */
- (void)registerServiceWithScope:(AnimaXServiceScope *)scope
                        protocol:(Protocol *)proto
                        withImpl:(id<AnimaXService>)impl;

/**
 * Retrieves a service implementation by protocol from the specified scope.
 *
 * @param scope The service scope
 * @param proto The protocol
 * @return The service implementation if found, nil otherwise
 */
- (nullable id<AnimaXService>)getServiceWithScope:(AnimaXServiceScope *)scope
                                         protocol:(Protocol *)proto;

/**
 * Unregisters a service from the specified scope by protocol.
 *
 * @param scope The service scope
 * @param proto The protocol to remove
 */
- (void)unregisterServiceWithScope:(AnimaXServiceScope *)scope protocol:(Protocol *)proto;

/**
 * Unregisters all services in the specified scope.
 *
 * @param scope The service scope
 */
- (void)unregisterAllServicesWithScope:(AnimaXServiceScope *)scope;

@end

NS_ASSUME_NONNULL_END
