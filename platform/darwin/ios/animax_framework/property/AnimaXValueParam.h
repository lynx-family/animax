// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Value parameter types supported by AnimaXValueParam
 */
typedef NS_ENUM(NSInteger, AnimaXValueParamType) {
  AnimaXValueParamTypeString,
  AnimaXValueParamTypeNumber,
  AnimaXValueParamTypeBoolean,
  AnimaXValueParamTypeCoordinate,
  AnimaXValueParamTypeColor,
  AnimaXValueParamTypeColorFilter,
  AnimaXValueParamTypeNull
};

/**
 * Apply mode for AnimaXValueParam.
 * SET: Overwrite the existing value.
 * ADD: Add the value to the existing value.
 *
 * Note: This is only applicable for coordinate or number values.
 */
typedef NS_ENUM(NSInteger, AnimaXValueApplyMode) {
  AnimaXValueApplyModeSet,
  AnimaXValueApplyModeAdd,
};

/**
 * Default frame index to apply the value to all frames.
 */
static const NSInteger kAnimaXValueFrameIndexAll = -1;

/**
 * A value parameter class that can hold different types of values (String, Number, Boolean, Point)
 * for animation parameters. Supports optional target frame for keyframe animations.
 *
 * Usage:
 *   AnimaXValueParam *stringParam = [AnimaXValueParam paramWithString:@"Hello"];
 *   AnimaXValueParam *numberParam = [AnimaXValueParam paramWithNumber:1.5];
 *   AnimaXValueParam *coordinateParam = [AnimaXValueParam paramWithCoordinateX:100 y:200 z:0];
 *   AnimaXValueParam *colorParam = [AnimaXValueParam paramWithColor:0xFF0000FF];
 *
 *   // With target frame for keyframe animations
 *   AnimaXValueParam *frameParam = [AnimaXValueParam paramWithString:@"Hello" targetFrame:30];
 */
@interface AnimaXValueParam : NSObject

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

#pragma mark - Factory Methods

/**
 * Creates an AnimaXValueParam from a String value
 * @param value String value
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithString:(NSString *)value;

/**
 * Creates an AnimaXValueParam from a String value with a target frame
 * @param value String value
 * @param targetFrame Frame number at which this value should be applied
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithString:(NSString *)value targetFrame:(NSInteger)targetFrame;

/**
 * Creates an AnimaXValueParam from a numeric value
 * @param value Numeric value
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithNumber:(double)value;

/**
 * Creates an AnimaXValueParam from a numeric value with a target frame
 * @param value Numeric value
 * @param targetFrame Frame number at which this value should be applied
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithNumber:(double)value targetFrame:(NSInteger)targetFrame;

/**
 * Creates an AnimaXValueParam from a numeric value with a target frame and apply mode
 * @param value Numeric value
 * @param applyMode Apply mode for this value
 * @param targetFrame Frame number at which this value should be applied
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithNumber:(double)value
                      applyMode:(AnimaXValueApplyMode)applyMode
                    targetFrame:(NSInteger)targetFrame;

/**
 * Creates an AnimaXValueParam from a boolean value
 * @param value Boolean value
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithBoolean:(BOOL)value;

/**
 * Creates an AnimaXValueParam from a boolean value with a target frame
 * @param value Boolean value
 * @param targetFrame Frame number at which this value should be applied
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithBoolean:(BOOL)value targetFrame:(NSInteger)targetFrame;

/**
 * Creates an AnimaXValueParam from a 2D coordinate (x,y)
 * @param x X coordinate
 * @param y Y coordinate
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithCoordinateX:(double)x y:(double)y;

/**
 * Creates an AnimaXValueParam from a 2D coordinate (x,y) with a target frame
 * @param x X coordinate
 * @param y Y coordinate
 * @param targetFrame Frame number at which this value should be applied
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithCoordinateX:(double)x y:(double)y targetFrame:(NSInteger)targetFrame;

/**
 * Creates an AnimaXValueParam from a 2D coordinate (x,y) with a target frame and apply mode
 * @param x X coordinate
 * @param y Y coordinate
 * @param applyMode Apply mode for this value
 * @param targetFrame Frame number at which this value should be applied
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithCoordinateX:(double)x
                                   y:(double)y
                           applyMode:(AnimaXValueApplyMode)applyMode
                         targetFrame:(NSInteger)targetFrame;

/**
 * Creates an AnimaXValueParam from a 3D coordinate (x,y,z)
 * @param x X coordinate
 * @param y Y coordinate
 * @param z Z coordinate
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithCoordinateX:(double)x y:(double)y z:(double)z;

/**
 * Creates an AnimaXValueParam from a coordinate (x, y, z) with a target frame
 * @param x X coordinate
 * @param y Y coordinate
 * @param z Z coordinate
 * @param targetFrame Frame number at which this value should be applied
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithCoordinateX:(double)x
                                   y:(double)y
                                   z:(double)z
                         targetFrame:(NSInteger)targetFrame;

/**
 * Creates an AnimaXValueParam from a coordinate (x, y, z) with a target frame and apply mode
 * @param x X coordinate
 * @param y Y coordinate
 * @param z Z coordinate
 * @param applyMode Apply mode for this value
 * @param targetFrame Frame number at which this value should be applied
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithCoordinateX:(double)x
                                   y:(double)y
                                   z:(double)z
                           applyMode:(AnimaXValueApplyMode)applyMode
                         targetFrame:(NSInteger)targetFrame;

/**
 * Creates an AnimaXValueParam from a color value
 * @param color Color integer value (ARGB format)
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithColor:(uint32_t)color;

/**
 * Creates an AnimaXValueParam from a color value with a target frame
 * @param color Color integer value (ARGB format)
 * @param targetFrame Frame number at which this value should be applied
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithColor:(uint32_t)color targetFrame:(NSInteger)targetFrame;

/**
 * Creates an AnimaXValueParam for a color filter with color and mode
 * @param color Color integer value (ARGB format)
 * @param mode Filter mode value
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithColorFilter:(uint32_t)color mode:(NSInteger)mode;

/**
 * Creates an AnimaXValueParam for a color filter with color and mode, and a target frame
 * @param color Color integer value (ARGB format)
 * @param mode Filter mode value
 * @param targetFrame Frame number at which this value should be applied
 * @return AnimaXValueParam instance
 */
+ (instancetype)paramWithColorFilter:(uint32_t)color
                                mode:(NSInteger)mode
                         targetFrame:(NSInteger)targetFrame;

/**
 * Creates a null AnimaXValueParam
 * @return AnimaXValueParam instance with null value
 */
+ (instancetype)nullParam;

#pragma mark - Properties

/**
 * The type of value stored in this parameter
 */
@property(nonatomic, readonly) AnimaXValueParamType type;

/**
 * Checks if this parameter has a target frame
 * @return YES if target frame is specified
 */
@property(nonatomic, readonly) BOOL hasTargetFrame;

/**
 * Gets the target frame if specified
 * @return Target frame or -1 if not specified
 */
@property(nonatomic, readonly) NSInteger targetFrame;

#pragma mark - Type Checking

/**
 * Checks if the value is a String
 * @return YES if value is a String
 */
@property(nonatomic, readonly) BOOL isString;

/**
 * Checks if the value is a Number
 * @return YES if value is a Number
 */
@property(nonatomic, readonly) BOOL isNumber;

/**
 * Checks if the value is a Boolean
 * @return YES if value is a Boolean
 */
@property(nonatomic, readonly) BOOL isBoolean;

/**
 * Checks if the value is a Coordinate
 * @return YES if value is a Coordinate
 */
@property(nonatomic, readonly) BOOL isCoordinate;

/**
 * Checks if the value is a Color
 * @return YES if value is a Color
 */
@property(nonatomic, readonly) BOOL isColor;

/**
 * Checks if the value is a ColorFilter
 * @return YES if value is a ColorFilter
 */
@property(nonatomic, readonly) BOOL isColorFilter;

/**
 * Checks if the value is null
 * @return YES if value is null
 */
@property(nonatomic, readonly) BOOL isNull;

#pragma mark - Value Access

/**
 * Gets the value as a String
 * @return String value or nil if not a String
 */
@property(nonatomic, readonly, nullable) NSString *stringValue;

/**
 * Gets the value as a Number
 * @return Number value or 0 if not a Number
 */
@property(nonatomic, readonly) double numberValue;

/**
 * Gets the value as a Boolean
 * @return Boolean value or NO if not a Boolean
 */
@property(nonatomic, readonly) BOOL booleanValue;

/**
 * Gets the X coordinate if this is a Coordinate value
 * @return X coordinate or 0 if not a Coordinate
 */
@property(nonatomic, readonly) double x;

/**
 * Gets the Y coordinate if this is a Coordinate value
 * @return Y coordinate or 0 if not a Coordinate
 */
@property(nonatomic, readonly) double y;

/**
 * Gets the color value
 * @return Color int value or 0 if not a Color or ColorFilter
 */
@property(nonatomic, readonly) uint32_t colorValue;

/**
 * Gets the filter mode value if this is a ColorFilter
 * @return Mode value or 0 if not a ColorFilter
 */
@property(nonatomic, readonly) NSInteger filterMode;

/**
 * Gets the Z coordinate if this is a Coordinate value
 * @return Z coordinate or 0 if not a Coordinate
 */
@property(nonatomic, readonly) double z;

/**
 * Apply mode for AnimaXValueParam.
 */
@property(nonatomic, readonly) AnimaXValueApplyMode applyMode;

@end

NS_ASSUME_NONNULL_END
