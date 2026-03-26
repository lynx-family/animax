// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "LottieValueDelegateAdapter.h"
#import <AnimaX/AnimaXFrameInfo.h>
#import <AnimaX/AnimaXValueCallback.h>
#import <AnimaX/AnimaXValueParam.h>
#import <CoreGraphics/CoreGraphics.h>
#import "LottieKeyPath.h"
#import "LottieValueDelegate.h"

@interface LottieColorValueCallback : AnimaXValueCallback
@property(nonatomic, strong) id<LottieColorValueDelegate> delegate;
@end

@interface LottieNumberValueCallback : AnimaXValueCallback
@property(nonatomic, strong) id<LottieNumberValueDelegate> delegate;
@end

@interface LottiePointValueCallback : AnimaXValueCallback
@property(nonatomic, strong) id<LottiePointValueDelegate> delegate;
@end

@interface LottieSizeValueCallback : AnimaXValueCallback
@property(nonatomic, strong) id<LottieSizeValueDelegate> delegate;
@end

@interface LottiePathValueCallback : AnimaXValueCallback
@property(nonatomic, strong) id<LottiePathValueDelegate> delegate;
@end

@implementation LottieColorValueCallback
- (AnimaXValueParam *)getValue:(AnimaXFrameInfo *)frameInfo {
  if (!self.delegate || !frameInfo) {
    return nil;
  }

  CGColorRef startColor = NULL;
  CGColorRef endColor = NULL;
  CGColorRef currentColor = NULL;

  if (frameInfo.startValue && frameInfo.startValue.isColor) {
    uint32_t colorValue = frameInfo.startValue.colorValue;
    CGFloat alpha = ((colorValue >> 24) & 0xFF) / 255.0f;
    CGFloat red = ((colorValue >> 16) & 0xFF) / 255.0f;
    CGFloat green = ((colorValue >> 8) & 0xFF) / 255.0f;
    CGFloat blue = (colorValue & 0xFF) / 255.0f;
    startColor = CGColorCreate(CGColorSpaceCreateDeviceRGB(), (CGFloat[]){red, green, blue, alpha});
  }

  if (frameInfo.endValue && frameInfo.endValue.isColor) {
    uint32_t colorValue = frameInfo.endValue.colorValue;
    CGFloat alpha = ((colorValue >> 24) & 0xFF) / 255.0f;
    CGFloat red = ((colorValue >> 16) & 0xFF) / 255.0f;
    CGFloat green = ((colorValue >> 8) & 0xFF) / 255.0f;
    CGFloat blue = (colorValue & 0xFF) / 255.0f;
    endColor = CGColorCreate(CGColorSpaceCreateDeviceRGB(), (CGFloat[]){red, green, blue, alpha});
  }

  CGColorRef resultColor = [self.delegate
             colorForFrame:frameInfo.overallProgress * (frameInfo.endFrame - frameInfo.startFrame) +
                           frameInfo.startFrame
             startKeyframe:frameInfo.startFrame
               endKeyframe:frameInfo.endFrame
      interpolatedProgress:frameInfo.overallProgress
                startColor:startColor
                  endColor:endColor
              currentColor:currentColor];

  if (startColor) {
    CGColorRelease(startColor);
  }
  if (endColor) {
    CGColorRelease(endColor);
  }
  if (currentColor) {
    CGColorRelease(currentColor);
  }

  if (resultColor) {
    const CGFloat *components = CGColorGetComponents(resultColor);
    CGFloat red = components[0];
    CGFloat green = components[1];
    CGFloat blue = components[2];
    CGFloat alpha = CGColorGetNumberOfComponents(resultColor) == 4 ? components[3] : 1.0;

    uint32_t colorValue = ((uint32_t)(alpha * 255) << 24) | ((uint32_t)(red * 255) << 16) |
                          ((uint32_t)(green * 255) << 8) | (uint32_t)(blue * 255);

    CGColorRelease(resultColor);
    return [AnimaXValueParam paramWithColor:colorValue];
  }

  return nil;
}
@end

@implementation LottieNumberValueCallback
- (AnimaXValueParam *)getValue:(AnimaXFrameInfo *)frameInfo {
  if (!self.delegate || !frameInfo) {
    return nil;
  }

  CGFloat startValue = 0;
  CGFloat endValue = 0;
  CGFloat currentValue = 0;

  if (frameInfo.startValue && frameInfo.startValue.isNumber) {
    startValue = frameInfo.startValue.numberValue;
  }

  if (frameInfo.endValue && frameInfo.endValue.isNumber) {
    endValue = frameInfo.endValue.numberValue;
  }

  CGFloat result = [self.delegate
        floatValueForFrame:frameInfo.overallProgress * (frameInfo.endFrame - frameInfo.startFrame) +
                           frameInfo.startFrame
             startKeyframe:frameInfo.startFrame
               endKeyframe:frameInfo.endFrame
      interpolatedProgress:frameInfo.overallProgress
                startValue:startValue
                  endValue:endValue
              currentValue:currentValue];

  return [AnimaXValueParam paramWithNumber:result];
}
@end

@implementation LottiePointValueCallback
- (AnimaXValueParam *)getValue:(AnimaXFrameInfo *)frameInfo {
  if (!self.delegate || !frameInfo) {
    return nil;
  }

  CGPoint startPoint = CGPointZero;
  CGPoint endPoint = CGPointZero;
  CGPoint currentPoint = CGPointZero;

  if (frameInfo.startValue && frameInfo.startValue.isCoordinate) {
    startPoint = CGPointMake(frameInfo.startValue.x, frameInfo.startValue.y);
  }

  if (frameInfo.endValue && frameInfo.endValue.isCoordinate) {
    endPoint = CGPointMake(frameInfo.endValue.x, frameInfo.endValue.y);
  }

  CGPoint result = [self.delegate
             pointForFrame:frameInfo.overallProgress * (frameInfo.endFrame - frameInfo.startFrame) +
                           frameInfo.startFrame
             startKeyframe:frameInfo.startFrame
               endKeyframe:frameInfo.endFrame
      interpolatedProgress:frameInfo.overallProgress
                startPoint:startPoint
                  endPoint:endPoint
              currentPoint:currentPoint];

  return [AnimaXValueParam paramWithCoordinateX:result.x y:result.y];
}
@end

@implementation LottieSizeValueCallback
- (AnimaXValueParam *)getValue:(AnimaXFrameInfo *)frameInfo {
  if (!self.delegate || !frameInfo) {
    return nil;
  }

  CGSize startSize = CGSizeZero;
  CGSize endSize = CGSizeZero;
  CGSize currentSize = CGSizeZero;

  if (frameInfo.startValue && frameInfo.startValue.isCoordinate) {
    startSize = CGSizeMake(frameInfo.startValue.x, frameInfo.startValue.y);
  }

  if (frameInfo.endValue && frameInfo.endValue.isCoordinate) {
    endSize = CGSizeMake(frameInfo.endValue.x, frameInfo.endValue.y);
  }

  CGSize result = [self.delegate
              sizeForFrame:frameInfo.overallProgress * (frameInfo.endFrame - frameInfo.startFrame) +
                           frameInfo.startFrame
             startKeyframe:frameInfo.startFrame
               endKeyframe:frameInfo.endFrame
      interpolatedProgress:frameInfo.overallProgress
                 startSize:startSize
                   endSize:endSize
               currentSize:currentSize];

  return [AnimaXValueParam paramWithCoordinateX:result.width y:result.height];
}
@end

@implementation LottiePathValueCallback
- (AnimaXValueParam *)getValue:(AnimaXFrameInfo *)frameInfo {
  if (!self.delegate || !frameInfo) {
    return nil;
  }

  CGPathRef resultPath = [self.delegate
              pathForFrame:frameInfo.overallProgress * (frameInfo.endFrame - frameInfo.startFrame) +
                           frameInfo.startFrame
             startKeyframe:frameInfo.startFrame
               endKeyframe:frameInfo.endFrame
      interpolatedProgress:frameInfo.overallProgress];

  // Path conversion would require additional implementation
  // For now, return null to use original path
  if (resultPath) {
    CGPathRelease(resultPath);
  }

  return [AnimaXValueParam nullParam];
}
@end

@implementation LottieValueDelegateAdapter

+ (AnimaXValueCallback *)createAnimaXValueCallbackWithLottieDelegate:
                             (id<LottieValueDelegate>)delegate
                                                          forKeyPath:(LottieKeyPath *)keyPath {
  if ([delegate conformsToProtocol:@protocol(LottieColorValueDelegate)]) {
    LottieColorValueCallback *callback = [[LottieColorValueCallback alloc] init];
    callback.delegate = (id<LottieColorValueDelegate>)delegate;
    return callback;
  } else if ([delegate conformsToProtocol:@protocol(LottieNumberValueDelegate)]) {
    LottieNumberValueCallback *callback = [[LottieNumberValueCallback alloc] init];
    callback.delegate = (id<LottieNumberValueDelegate>)delegate;
    return callback;
  } else if ([delegate conformsToProtocol:@protocol(LottiePointValueDelegate)]) {
    LottiePointValueCallback *callback = [[LottiePointValueCallback alloc] init];
    callback.delegate = (id<LottiePointValueDelegate>)delegate;
    return callback;
  } else if ([delegate conformsToProtocol:@protocol(LottieSizeValueDelegate)]) {
    LottieSizeValueCallback *callback = [[LottieSizeValueCallback alloc] init];
    callback.delegate = (id<LottieSizeValueDelegate>)delegate;
    return callback;
  } else if ([delegate conformsToProtocol:@protocol(LottiePathValueDelegate)]) {
    LottiePathValueCallback *callback = [[LottiePathValueCallback alloc] init];
    callback.delegate = (id<LottiePathValueDelegate>)delegate;
    return callback;
  }

  return nil;
}

+ (LayerPropertyType)getLayerPropertyTypeForKeyPath:(LottieKeyPath *)keyPath {
  if (!keyPath || keyPath.keys.count == 0) {
    return LayerPropertyTypeUnknown;
  }

  NSArray<NSString *> *keys = keyPath.keys;
  NSString *lastKey = [keys lastObject];
  NSString *secondLastKey = keys.count >= 2 ? keys[keys.count - 2] : nil;

  // Color related
  if ([lastKey isEqualToString:@"Color"] || [lastKey isEqualToString:@"Fill"]) {
    return LayerPropertyTypeColor;
  }
  if ([lastKey isEqualToString:@"Stroke Color"]) {
    return LayerPropertyTypeStrokeColor;
  }
  if ([lastKey isEqualToString:@"Stroke Width"]) {
    return LayerPropertyTypeStrokeWidth;
  }

  // Transform related
  if ([lastKey isEqualToString:@"Position"]) {
    return LayerPropertyTypeTransformPosition;
  }
  if ([lastKey isEqualToString:@"Scale"]) {
    return LayerPropertyTypeTransformScale;
  }
  if ([lastKey isEqualToString:@"Rotation"]) {
    return LayerPropertyTypeTransformRotation;
  }
  if ([lastKey isEqualToString:@"Anchor Point"]) {
    return LayerPropertyTypeTransformAnchor;
  }
  if ([lastKey isEqualToString:@"Opacity"]) {
    return LayerPropertyTypeTransformOpacity;
  }

  // Size related
  if ([lastKey isEqualToString:@"Size"]) {
    return LayerPropertyTypeEllipseSize;
  }

  // Text related
  if ([lastKey isEqualToString:@"Text"]) {
    return LayerPropertyTypeTextValue;
  }
  if ([lastKey isEqualToString:@"Text Size"]) {
    return LayerPropertyTypeTextSize;
  }
  if ([lastKey isEqualToString:@"Text Color"]) {
    return LayerPropertyTypeTextColor;
  }

  // Path related
  if ([lastKey isEqualToString:@"Path"]) {
    return LayerPropertyTypeUnknown;  // Special handling for path
  }

  // Transform with prefix
  if ([secondLastKey isEqualToString:@"Transform"]) {
    if ([lastKey isEqualToString:@"Position"]) {
      return LayerPropertyTypeTransformPosition;
    } else if ([lastKey isEqualToString:@"Scale"]) {
      return LayerPropertyTypeTransformScale;
    } else if ([lastKey isEqualToString:@"Rotation"]) {
      return LayerPropertyTypeTransformRotation;
    } else if ([lastKey isEqualToString:@"Anchor"]) {
      return LayerPropertyTypeTransformAnchor;
    } else if ([lastKey isEqualToString:@"Opacity"]) {
      return LayerPropertyTypeTransformOpacity;
    }
  }

  return LayerPropertyTypeUnknown;
}

+ (NSArray<NSString *> *)trimmedKeysForKeyPath:(LottieKeyPath *)keyPath
                                  propertyType:(LayerPropertyType)propertyType {
  if (!keyPath || keyPath.keys.count == 0) {
    return keyPath.keys ?: @[];
  }

  NSMutableArray *trimmedKeys = [NSMutableArray arrayWithArray:keyPath.keys];

  // Remove the last key if it matches property type
  if (propertyType == LayerPropertyTypeColor || propertyType == LayerPropertyTypeStrokeColor ||
      propertyType == LayerPropertyTypeStrokeWidth ||
      propertyType == LayerPropertyTypeTransformPosition ||
      propertyType == LayerPropertyTypeTransformScale ||
      propertyType == LayerPropertyTypeTransformRotation ||
      propertyType == LayerPropertyTypeTransformAnchor ||
      propertyType == LayerPropertyTypeTransformOpacity ||
      propertyType == LayerPropertyTypeEllipseSize || propertyType == LayerPropertyTypeTextValue ||
      propertyType == LayerPropertyTypeTextSize || propertyType == LayerPropertyTypeTextColor) {
    if (trimmedKeys.count > 0) {
      [trimmedKeys removeLastObject];
    }
  }

  // Special handling for Transform prefixed properties
  if (trimmedKeys.count >= 2) {
    NSString *secondLastKey = trimmedKeys[trimmedKeys.count - 2];
    NSString *lastKey = trimmedKeys.lastObject;
    if ([secondLastKey isEqualToString:@"Transform"] &&
        ([lastKey isEqualToString:@"Position"] || [lastKey isEqualToString:@"Scale"] ||
         [lastKey isEqualToString:@"Rotation"] || [lastKey isEqualToString:@"Anchor"] ||
         [lastKey isEqualToString:@"Opacity"])) {
      // Remove "Position", "Scale", etc.
      [trimmedKeys removeLastObject];
      // Remove "Transform"
      [trimmedKeys removeLastObject];
    }
  }

  return [trimmedKeys copy];
}

@end
