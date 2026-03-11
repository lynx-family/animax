// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/LayerPropertyType.h>
#import <AnimaX/ResourcePropertyType.h>
#import <Foundation/Foundation.h>

@class AnimaXKeyPath;
@class AnimaXValueParam;
@class AnimaXValueCallback;
@protocol AnimaXPropertyCallback;
@protocol AnimaXKeyPathListCallback;

NS_ASSUME_NONNULL_BEGIN

/**
 * iOS platform property updater that handles all property update operations.
 *
 * This class encapsulates the logic for updating layer properties, resource properties,
 * and managing property callbacks on iOS platform.
 */
@interface AnimaXPropertyUpdater : NSObject

/**
 * Initialize property updater with weak reference to player
 * @param playerPtr Shared pointer to the AnimaX player
 * @return Initialized property updater instance
 */
- (instancetype)initWithPlayer:(void *)playerPtr;

/**
 * Update a layer property with static value
 * @param keyPath Target key path for the property
 * @param propertyType Type of the layer property
 * @param value New value for the property
 * @param callback Callback for operation result (nullable)
 */
- (void)updateLayerProperty:(AnimaXKeyPath *)keyPath
               propertyType:(LayerPropertyType)propertyType
                      value:(AnimaXValueParam *)value
                   callback:(nullable id<AnimaXPropertyCallback>)callback;

/**
 * Update a resource property
 * @param keyPath Target key path for the resource
 * @param propertyType Type of the resource property
 * @param value New value for the property
 * @param callback Callback for operation result (nullable)
 */
- (void)updateResourceProperty:(AnimaXKeyPath *)keyPath
                  propertyType:(ResourcePropertyType)propertyType
                         value:(AnimaXValueParam *)value
                      callback:(nullable id<AnimaXPropertyCallback>)callback;

/**
 * Add a dynamic property callback for layer property
 * @param propertyType Type of the layer property
 * @param keyPath Target key path for the property
 * @param valueCallback Dynamic value callback
 * @param callback Callback for operation result (nullable)
 */
- (void)addLayerPropertyCallback:(LayerPropertyType)propertyType
                         keyPath:(AnimaXKeyPath *)keyPath
                   valueCallback:(AnimaXValueCallback *)valueCallback
                        callback:(nullable id<AnimaXPropertyCallback>)callback;

/**
 * Get all descendant keys for a keypath.
 * @param keyPath The keypath to get keys for.
 * @param callback Callback for operation result (nullable)
 */
- (void)getKeysForKeyPath:(AnimaXKeyPath *)keyPath
                 callback:(nullable id<AnimaXKeyPathListCallback>)callback;

@end

NS_ASSUME_NONNULL_END
