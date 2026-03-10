// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

#import <AnimaX/AnimaXComposition.h>
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXSurfaceDrawable.h>
#import <AnimaX/LayerPropertyType.h>
#import <AnimaX/ResourcePropertyType.h>

@class AnimaXKeyPath;
@class AnimaXValueParam;
@class AnimaXValueCallback;
@protocol AnimaXPropertyCallback;
@protocol AnimaXKeyPathListCallback;

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, VisibilityState) {
  // Changed when app enter foreground or background
  kBackground = 1,
  // Changed when platform view's setHidden called
  kVisible = 1 << 1,
  // Changed when platform layer's setOpactiy called
  kOpacity = 1 << 2,
  // Changed when platform view's width or height update
  kSize = 1 << 3,
  // Changed when platform view's attach or detach from window
  kAttach = 1 << 4,
  // Changed when platform view's parent visibility is changed, affecting this
  // view's aggregated visibility state. Represents the true visibility of a view based
  // on the entire hierarchy - the view is only considered visible when both itself
  // and all ancestors are visible and within screen bounds.
  kAggregated = 1 << 5
};

@protocol AnimaXPlayerProtocol <NSObject>

// Resource-loading
- (void)setImageFolder:(NSString *)imageFolder;
- (void)setComposition:(AnimaXComposition *)composition;
- (void)setSrc:(NSString *)src;
- (void)setPolyfill:(NSDictionary *)polyfill;
- (void)setJson:(NSString *)json;
- (void)setDynamicResource:(BOOL)enable;

// Exclusively for client-side operations. Param bundle will be used to read subordinate resources.
- (void)setSrc:(NSString *)src inBundle:(NSBundle *)bundle;
- (void)setJson:(NSString *)json inBundle:(NSBundle *)bundle;

// Animation-playing(setProperties)
- (void)setLoop:(BOOL)enable;
- (void)setStartFrame:(double)startFrame;
- (void)setEndFrame:(double)endFrame;
- (void)setAutoReverse:(BOOL)enable;
- (void)setProgress:(double)progress;
- (void)setLoopCount:(int)loopCount;
- (void)setObjectfit:(NSString *)objectfit;
- (void)setObjectPosition:(NSString *)objectPosition;
- (void)setAutoplay:(BOOL)enable;
- (void)setSpeed:(double)speed;
- (void)setMaxFrameRate:(double)maxFrameRate;

// Animation-playing(commands)
- (void)play;
- (void)playFrom:(double)startFrame to:(double)endFrame;
- (void)resume;
- (void)stop;
- (void)pause;
- (void)seekTo:(double)frame;
- (void)reload;

// Animation-playing(getInfo)
- (double)durationInMS;
- (BOOL)isAnimating;
- (double)currentFrame;
- (int64_t)memoryUsageBytes;

// Surface
- (void)adoptAnimaXSurfaceDrawable:(AnimaXSurfaceDrawable *)drawable;

// Event(Touch)
- (void)handleTapAtPoint:(CGPoint)point;

// Event(Listening)
- (void)setFPSEventInterval:(long)interval;

- (void)subscribeUpdateEvent:(int)frame;
- (void)unsubscribeUpdateEvent:(int)frame;
- (void)subscribeUpdateEvents:(NSArray *)frames subscribe:(BOOL)subscribe;

- (void)addAnimationEventListener:(id<AnimaXAnimationListener>)listener;
- (void)removeAnimationEventListener:(id<AnimaXAnimationListener>)listener;

// Event(visibility)
- (void)updateVisibilityState:(BOOL)visible forType:(VisibilityState)state;
- (void)onHide:(VisibilityState)state;
- (void)onShow:(VisibilityState)state;
- (void)enterForeground;
- (void)enterBackground;

- (AnimaXContext *)getAnimaXContext;

/**
 * Update a layer property with a specific value and optional callback
 * @param keyPath The key path identifying the layer and property to update
 * @param propertyType The type of layer property to update
 * @param value The new value for the property
 * @param callback Optional callback to be notified of the operation result
 */
- (void)updateLayerProperty:(AnimaXKeyPath *)keyPath
               propertyType:(LayerPropertyType)propertyType
                      value:(AnimaXValueParam *)value
                   callback:(nullable id<AnimaXPropertyCallback>)callback;

/**
 * Update a resource property with a specific value and optional callback
 * @param keyPath The key path identifying the resource to update
 * @param propertyType The type of resource property to update
 * @param value The new value for the property
 * @param callback Optional callback to be notified of the operation result
 */
- (void)updateResourceProperty:(AnimaXKeyPath *)keyPath
                  propertyType:(ResourcePropertyType)propertyType
                         value:(AnimaXValueParam *)value
                      callback:(nullable id<AnimaXPropertyCallback>)callback;

/**
 * Add a value callback to dynamically modify layer property values
 * @param propertyType The type of layer property
 * @param keyPath The key path identifying the layer and property
 * @param valueCallback The value callback to add
 * @param callback Optional callback to be notified of the operation result
 */
- (void)addLayerPropertyCallback:(LayerPropertyType)propertyType
                         keyPath:(AnimaXKeyPath *)keyPath
                   valueCallback:(AnimaXValueCallback *)valueCallback
                        callback:(nullable id<AnimaXPropertyCallback>)callback;

/**
 * Get all descendant keys for a given keyPath
 * @param keyPath The key path to search for
 * @param callback Callback to be notified with the list of AnimaXKeyPath objects
 */
- (void)getKeysForKeyPath:(AnimaXKeyPath *)keyPath callback:(id<AnimaXKeyPathListCallback>)callback;

@end

NS_ASSUME_NONNULL_END
