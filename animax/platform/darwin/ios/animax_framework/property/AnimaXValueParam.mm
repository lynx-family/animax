// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXValueParam.h>
#import "AnimaXValueParam+Internal.h"

#include "include/property/animax_value_param.h"
#include "src/model/value/base_value.h"

// Define a struct to hold 3D coordinates
struct AnimaXCoordinate {
  double x, y, z;
};

@interface AnimaXValueParam ()

@property(nonatomic, assign) AnimaXValueParamType type;
@property(nonatomic, strong, nullable) NSString *internalStringValue;
@property(nonatomic, assign) double internalNumberValue;
@property(nonatomic, assign) BOOL internalBooleanValue;
@property(nonatomic, assign) struct AnimaXCoordinate internalCoordinateValue;
@property(nonatomic, assign) uint32_t internalColorValue;
@property(nonatomic, assign) NSInteger internalFilterMode;
@property(nonatomic, assign) NSInteger internalTargetFrame;

@end

@implementation AnimaXValueParam

#pragma mark - Initialization

- (instancetype)initWithType:(AnimaXValueParamType)type targetFrame:(NSInteger)targetFrame {
  self = [super init];
  if (self) {
    _type = type;
    _internalTargetFrame = targetFrame;
    _internalNumberValue = 0.0;
    _internalBooleanValue = NO;
    _internalCoordinateValue = (struct AnimaXCoordinate){0, 0, 0};
    _internalColorValue = 0;
    _internalFilterMode = 0;
  }
  return self;
}

#pragma mark - Factory Methods

+ (instancetype)paramWithString:(NSString *)value {
  return [self paramWithString:value targetFrame:lynx::animax::AnimaXValueParam::kFrameIndexAll];
}

+ (instancetype)paramWithString:(NSString *)value targetFrame:(NSInteger)targetFrame {
  AnimaXValueParam *param = [[AnimaXValueParam alloc] initWithType:AnimaXValueParamTypeString
                                                       targetFrame:targetFrame];
  param.internalStringValue = [value copy];
  return param;
}

+ (instancetype)paramWithNumber:(double)value {
  return [self paramWithNumber:value targetFrame:lynx::animax::AnimaXValueParam::kFrameIndexAll];
}

+ (instancetype)paramWithNumber:(double)value targetFrame:(NSInteger)targetFrame {
  AnimaXValueParam *param = [[AnimaXValueParam alloc] initWithType:AnimaXValueParamTypeNumber
                                                       targetFrame:targetFrame];
  param.internalNumberValue = value;
  return param;
}

+ (instancetype)paramWithBoolean:(BOOL)value {
  return [self paramWithBoolean:value targetFrame:lynx::animax::AnimaXValueParam::kFrameIndexAll];
}

+ (instancetype)paramWithBoolean:(BOOL)value targetFrame:(NSInteger)targetFrame {
  AnimaXValueParam *param = [[AnimaXValueParam alloc] initWithType:AnimaXValueParamTypeBoolean
                                                       targetFrame:targetFrame];
  param.internalBooleanValue = value;
  return param;
}

+ (instancetype)paramWithCoordinateX:(double)x y:(double)y {
  return [self paramWithCoordinateX:x
                                  y:y
                        targetFrame:lynx::animax::AnimaXValueParam::kFrameIndexAll];
}

+ (instancetype)paramWithCoordinateX:(double)x y:(double)y targetFrame:(NSInteger)targetFrame {
  return [self paramWithCoordinateX:x y:y z:0 targetFrame:targetFrame];
}

+ (instancetype)paramWithCoordinateX:(double)x y:(double)y z:(double)z {
  return [self paramWithCoordinateX:x
                                  y:y
                                  z:z
                        targetFrame:lynx::animax::AnimaXValueParam::kFrameIndexAll];
}

+ (instancetype)paramWithCoordinateX:(double)x
                                   y:(double)y
                                   z:(double)z
                         targetFrame:(NSInteger)targetFrame {
  AnimaXValueParam *param = [[AnimaXValueParam alloc] initWithType:AnimaXValueParamTypeCoordinate
                                                       targetFrame:targetFrame];
  param.internalCoordinateValue = (struct AnimaXCoordinate){x, y, z};
  return param;
}

+ (instancetype)paramWithColor:(uint32_t)color {
  return [self paramWithColor:color targetFrame:lynx::animax::AnimaXValueParam::kFrameIndexAll];
}

+ (instancetype)paramWithColor:(uint32_t)color targetFrame:(NSInteger)targetFrame {
  AnimaXValueParam *param = [[AnimaXValueParam alloc] initWithType:AnimaXValueParamTypeColor
                                                       targetFrame:targetFrame];
  param.internalColorValue = color;
  return param;
}

+ (instancetype)paramWithColorFilter:(uint32_t)color mode:(NSInteger)mode {
  return [self paramWithColorFilter:color
                               mode:mode
                        targetFrame:lynx::animax::AnimaXValueParam::kFrameIndexAll];
}

+ (instancetype)paramWithColorFilter:(uint32_t)color
                                mode:(NSInteger)mode
                         targetFrame:(NSInteger)targetFrame {
  AnimaXValueParam *param = [[AnimaXValueParam alloc] initWithType:AnimaXValueParamTypeColorFilter
                                                       targetFrame:targetFrame];
  param.internalColorValue = color;
  param.internalFilterMode = mode;
  return param;
}

+ (instancetype)nullParam {
  return [[AnimaXValueParam alloc] initWithType:AnimaXValueParamTypeNull
                                    targetFrame:lynx::animax::AnimaXValueParam::kFrameIndexAll];
}

#pragma mark - Properties

- (BOOL)hasTargetFrame {
  return _internalTargetFrame != lynx::animax::AnimaXValueParam::kFrameIndexAll;
}

- (NSInteger)targetFrame {
  return _internalTargetFrame;
}

#pragma mark - Type Checking

- (BOOL)isString {
  return _type == AnimaXValueParamTypeString;
}

- (BOOL)isNumber {
  return _type == AnimaXValueParamTypeNumber;
}

- (BOOL)isBoolean {
  return _type == AnimaXValueParamTypeBoolean;
}

- (BOOL)isCoordinate {
  return _type == AnimaXValueParamTypeCoordinate;
}

- (BOOL)isColor {
  return _type == AnimaXValueParamTypeColor;
}

- (BOOL)isColorFilter {
  return _type == AnimaXValueParamTypeColorFilter;
}

- (BOOL)isNull {
  return _type == AnimaXValueParamTypeNull;
}

#pragma mark - Value Access

- (NSString *)stringValue {
  return self.isString ? _internalStringValue : nil;
}

- (double)numberValue {
  return self.isNumber ? _internalNumberValue : 0.0;
}

- (BOOL)booleanValue {
  return self.isBoolean ? _internalBooleanValue : NO;
}

- (double)x {
  return self.isCoordinate ? _internalCoordinateValue.x : 0.0;
}

- (double)y {
  return self.isCoordinate ? _internalCoordinateValue.y : 0.0;
}

- (double)z {
  return self.isCoordinate ? _internalCoordinateValue.z : 0.0;
}

- (uint32_t)colorValue {
  if (self.isColor || self.isColorFilter) {
    return _internalColorValue;
  }
  return 0;
}

- (NSInteger)filterMode {
  return self.isColorFilter ? _internalFilterMode : 0;
}

#pragma mark - NSObject

- (NSString *)description {
  switch (_type) {
    case AnimaXValueParamTypeString:
      return [NSString stringWithFormat:@"AnimaXValueParam{stringValue='%@', targetFrame=%ld}",
                                        _internalStringValue, (long)_internalTargetFrame];
    case AnimaXValueParamTypeNumber:
      return [NSString stringWithFormat:@"AnimaXValueParam{numberValue=%f, targetFrame=%ld}",
                                        _internalNumberValue, (long)_internalTargetFrame];
    case AnimaXValueParamTypeBoolean:
      return [NSString stringWithFormat:@"AnimaXValueParam{booleanValue=%@, targetFrame=%ld}",
                                        _internalBooleanValue ? @"YES" : @"NO",
                                        (long)_internalTargetFrame];
    case AnimaXValueParamTypeCoordinate:
      return [NSString
          stringWithFormat:@"AnimaXValueParam{coordinateValue=(%f,%f,%f), targetFrame=%ld}",
                           _internalCoordinateValue.x, _internalCoordinateValue.y,
                           _internalCoordinateValue.z, (long)_internalTargetFrame];
    case AnimaXValueParamTypeColor:
      return [NSString stringWithFormat:@"AnimaXValueParam{colorValue=%u, targetFrame=%ld}",
                                        _internalColorValue, (long)_internalTargetFrame];
    case AnimaXValueParamTypeColorFilter:
      return
          [NSString stringWithFormat:@"AnimaXValueParam{colorValue=%u, mode=%ld, targetFrame=%ld}",
                                     _internalColorValue, (long)_internalFilterMode,
                                     (long)_internalTargetFrame];
    case AnimaXValueParamTypeNull:
      return [NSString
          stringWithFormat:@"AnimaXValueParam{null, targetFrame=%ld}", (long)_internalTargetFrame];
    default:
      return @"AnimaXValueParam{unknown}";
  }
}

@end

@implementation AnimaXValueParam (Internal)

- (std::unique_ptr<lynx::animax::AnimaXValueParam>)toNative {
  int32_t targetFrame = [self hasTargetFrame] ? static_cast<int32_t>([self targetFrame])
                                              : lynx::animax::AnimaXValueParam::kFrameIndexAll;

  switch (self.type) {
    case AnimaXValueParamTypeString: {
      std::string stringValue = self.stringValue ? [self.stringValue UTF8String] : "";
      return std::make_unique<lynx::animax::AnimaXValueParam>(stringValue, targetFrame);
    }
    case AnimaXValueParamTypeNumber: {
      return std::make_unique<lynx::animax::AnimaXValueParam>([self numberValue], targetFrame);
    }
    case AnimaXValueParamTypeBoolean: {
      return std::make_unique<lynx::animax::AnimaXValueParam>([self booleanValue], targetFrame);
    }
    case AnimaXValueParamTypeCoordinate: {
      return std::make_unique<lynx::animax::AnimaXValueParam>([self x], [self y], [self z],
                                                              targetFrame);
    }
    case AnimaXValueParamTypeColor: {
      return std::make_unique<lynx::animax::AnimaXValueParam>(
          static_cast<int32_t>([self colorValue]), targetFrame);
    }
    case AnimaXValueParamTypeColorFilter: {
      lynx::animax::AnimaXValueParam::ColorFilterParam params{
          static_cast<int32_t>([self colorValue]), static_cast<int32_t>([self filterMode])};
      return std::make_unique<lynx::animax::AnimaXValueParam>(params, targetFrame);
    }
    case AnimaXValueParamTypeNull:
    default:
      return std::make_unique<lynx::animax::AnimaXValueParam>();
  }
}

+ (nullable instancetype)valueParamWithNativeValue:(const lynx::animax::Value *)value {
  if (!value) {
    return nil;
  }

  // Use the base class conversion to create an AnimaXValueParam from Value
  auto valueParam = lynx::animax::AnimaXValueParam::FromValue(value);
  if (!valueParam) {
    return nil;
  }

  return [self valueParamWithNative:valueParam.get()];
}

+ (nullable instancetype)valueParamWithNative:(const lynx::animax::AnimaXValueParam *)valueParam {
  if (!valueParam) {
    return nil;
  }

  // Convert the native AnimaXValueParam to iOS AnimaXValueParam based on its type
  switch (valueParam->GetType()) {
    case lynx::animax::AnimaXValueParam::Type::kString:
      return [AnimaXValueParam paramWithString:@(valueParam->GetStringValue().c_str())];
    case lynx::animax::AnimaXValueParam::Type::kNumber:
      return [AnimaXValueParam paramWithNumber:valueParam->GetNumberValue()];
    case lynx::animax::AnimaXValueParam::Type::kBoolean:
      return [AnimaXValueParam paramWithBoolean:valueParam->GetBooleanValue()];
    case lynx::animax::AnimaXValueParam::Type::kCoordinate:
      return [AnimaXValueParam paramWithCoordinateX:valueParam->GetX()
                                                  y:valueParam->GetY()
                                                  z:valueParam->GetZ()];
    case lynx::animax::AnimaXValueParam::Type::kColor:
      return [AnimaXValueParam paramWithColor:valueParam->GetColorValue()];
    case lynx::animax::AnimaXValueParam::Type::kColorFilter:
      return [AnimaXValueParam
          paramWithColorFilter:valueParam->GetColorValue()
                          mode:static_cast<NSInteger>(valueParam->GetNumberValue())];
    case lynx::animax::AnimaXValueParam::Type::kNull:
    default:
      return [AnimaXValueParam nullParam];
  }
}

@end
