// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Enumeration of layer property types that can be animated.
 * These values correspond to the LayerPropertyType enum in the native code.
 */
typedef NS_ENUM(NSUInteger, LayerPropertyType) {
  /**
   * Unknown property type.
   * Type: Unknown
   */
  LayerPropertyTypeUnknown = 0,

  /**
   * The layer's visibility.
   * Type: Integer - when the value is 1, the layer is visible.
   */
  LayerPropertyTypeVisibility = 1,

  /**
   * The layer's opacity transform.
   * Type: Integer - value is between 0 and 100.
   */
  LayerPropertyTypeTransformOpacity,

  /**
   * The layer's anchor transform.
   * Type: Point
   */
  LayerPropertyTypeTransformAnchor,

  /**
   * The layer's position transform.
   * Type: Point
   */
  LayerPropertyTypeTransformPosition,

  /**
   * The layer's scale transform.
   * Type: Scale
   */
  LayerPropertyTypeTransformScale,

  /**
   * The layer's 2D rotation transform.
   * Type: Float
   */
  LayerPropertyTypeTransformRotation,

  /**
   * The layer's skew transform.
   * Type: Float
   */
  LayerPropertyTypeTransformSkew,

  /**
   * The layer's skew angle transform.
   * Type: Float
   */
  LayerPropertyTypeTransformSkewAngle,

  /**
   * The layer's start opacity transform.
   * Type: Float
   */
  LayerPropertyTypeTransformStartOpacity,

  /**
   * The layer's end opacity transform.
   * Type: Float
   */
  LayerPropertyTypeTransformEndOpacity,

  /**
   * The layer's rotation x transform.
   * Type: Float
   */
  LayerPropertyTypeTransformRotationX,

  /**
   * The layer's 3D rotation y transform.
   * Type: Float
   */
  LayerPropertyTypeTransformRotationY,

  /**
   * The layer's rotation z transform.
   * Type: Float
   */
  LayerPropertyTypeTransformRotationZ,

  /**
   * The text layer's text value.
   * Type: String
   */
  LayerPropertyTypeTextValue = 101,

  /**
   * The text layer's text size transform.
   * Type: Float
   */
  LayerPropertyTypeTextSize,

  /**
   * The text layer's text color transform.
   * Type: Color
   */
  LayerPropertyTypeTextColor,

  /**
   * The text layer's text tracking transform.
   * Type: Float
   */
  LayerPropertyTypeTextTracking,

  /**
   * The layer or content's color.
   * Type: Color
   */
  LayerPropertyTypeColor = 201,

  /**
   * The layer or content's color filter.
   * Type: ColorFilter
   */
  LayerPropertyTypeColorFilter,

  /**
   * The layer or content's stroke color.
   * Type: Color
   */
  LayerPropertyTypeStrokeColor,

  /**
   * The layer or content's stroke width.
   * Type: Float
   */
  LayerPropertyTypeStrokeWidth,

  /**
   * The content's opacity.
   * Type: Integer
   */
  LayerPropertyTypeOpacity,

  /**
   * The content's blur radius.
   * Type: Float
   */
  LayerPropertyTypeBlurRadius,

  /**
   * The content's ellipse size.
   * Type: Point
   */
  LayerPropertyTypeEllipseSize,

  /**
   * The content's rectangle size.
   * Type: Point
   */
  LayerPropertyTypeRectangleSize,

  /**
   * The content's corner radius.
   * Type: Float
   */
  LayerPropertyTypeCornerRadius,

  /**
   * The content's position.
   * Type: Point
   */
  LayerPropertyTypePosition,

  /**
   * The content's repeater copies.
   * Type: Float
   */
  LayerPropertyTypeRepeaterCopies,

  /**
   * The content's repeater offset.
   * Type: Point
   */
  LayerPropertyTypeRepeaterOffset,

  /**
   * The content's polygon star points.
   * Type: Float
   */
  LayerPropertyTypePolyStarPoints,

  /**
   * The content's polygon star rotation.
   * Type: Float
   */
  LayerPropertyTypePolyStarRotation,

  /**
   * The content's polygon star inner radius.
   * Type: Float
   */
  LayerPropertyTypePolyStarInnerRadius,

  /**
   * The content's polygon star outer radius.
   * Type: Float
   */
  LayerPropertyTypePolyStarOuterRadius,

  /**
   * The content's polygon star inner rounded.
   * Type: Float
   */
  LayerPropertyTypePolyStarInnerRounded,

  /**
   * The content's polygon star outer rounded.
   * Type: Float
   */
  LayerPropertyTypePolyStarOuterRounded,

  /**
   * The content's drop shadow color.
   * Type: Color
   */
  LayerPropertyTypeDropShadowColor,

  /**
   * The content's drop shadow opacity.
   * Type: Integer
   */
  LayerPropertyTypeDropShadowOpacity,

  /**
   * The content's drop shadow direction.
   * Type: Float
   */
  LayerPropertyTypeDropShadowDirection,

  /**
   * The content's drop shadow distance.
   * Type: Float
   */
  LayerPropertyTypeDropShadowDistance,

  /**
   * The content's drop shadow radius.
   * Type: Float
   */
  LayerPropertyTypeDropShadowRadius,
};

NS_ASSUME_NONNULL_END
