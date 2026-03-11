// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_PROPERTY_TYPE_H_
#define ANIMAX_SRC_PROPERTY_PROPERTY_TYPE_H_

#include <cstdint>

namespace lynx {
namespace animax {

static constexpr uint16_t kTransformRangeStart = 2;
static constexpr uint16_t kTextRangeStart = 101;
static constexpr uint16_t kContentRangeStart = 201;

static constexpr uint16_t kImageRangeStart = 1;
static constexpr uint16_t kFontRangeStart = 101;
static constexpr uint16_t kVideoRangeStart = 201;
static constexpr uint16_t kAudioRangeStart = 301;

enum class LayerPropertyType : uint16_t {
  kUnknown = 0,
  // The layer's visibility, the type is Integer, when the value is 1, the layer
  // is visible.
  kVisibility = 1,

  // The layer's opacity transform, the type is Integer, the value is bettwen 0
  // and 100.
  kTransformOpacity = kTransformRangeStart,
  // The layer's anchor transform, the type is PointF.
  kTransformAnchor,
  // The layer's position transform, the type is PointF.
  kTransformPosition,
  // The layer's scale transform, the type is ScaleF.
  kTransformScale,
  // The layer's 2D rotation transform, the type is Float.
  kTransformRotation,
  // The layer's skew transform, the type is Float.
  kTransformSkew,
  // The layer's skew angle transform, the type is Float.
  kTransformSkewAngle,
  // The layer's start opacity transform, the type is Float.
  kTransformStartOpacity,
  // The layer's end opacity transform, the type is Float.
  kTransformEndOpacity,
  // The layer's rotation x transform, the type is Float.
  kTransformRotationX,
  // The layer's 3D rotation y transform, the type is Float.
  kTransformRotationY,
  // The layer's rotation z transform, the type is Float.
  kTransformRotationZ,

  // The text layer's text value, the type is String.
  kTextValue = kTextRangeStart,
  // The text layer's text size transform, the type is Float.
  kTextSize,
  // The text layer's text color transform, the type is Color.
  kTextColor,
  // The text layer's text tracking transform, the type is Float.
  kTextTracking,

  // The layer or content's color, the type is Color.
  kColor = kContentRangeStart,
  // The layer or content's color filter, the type is ColorFilter.
  kColorFilter,
  // The layer or content's stroke color, the type is Color.
  kStrokeColor,
  // The layer or content's stroke width, the type is Float.
  kStrokeWidth,
  // The content's opacity, the type is Integer.
  kOpacity,
  // The content's blur radius, the type is Float.
  kBlurRadius,
  // The content's ellipse size, the type is PointF.
  kEllipseSize,
  // The content's rectangle size, the type is PointF.
  kRectangleSize,
  // The content's corner radius, the type is Float.
  kCornerRadius,
  // The content's position, the type is PointF.
  kPosition,
  // The content's repeater copies, the type is Float.
  kRepeaterCopies,
  // The content's repeater offset, the type is PointF.
  kRepeaterOffset,
  // The content's polygon star points, the type is Float.
  kPolyStarPoints,
  // The content's polygon star rotation, the type is Float.
  kPolyStarRotation,
  // The content's polygon star inner radius, the type is Float.
  kPolyStarInnerRadius,
  // The content's polygon star outer radius, the type is Float.
  kPolyStarOuterRadius,
  // The content's polygon star inner rounded, the type is Float.
  kPolyStarInnerRounded,
  // The content's polygon star outer rounded, the type is Float.
  kPolyStarOuterRounded,
  // The content's drop shadow color, the type is Color.
  kDropShadowColor,
  // The content's drop shadow opacity, the type is Integer.
  kDropShadowOpacity,
  // The content's drop shadow direction, the type is Float.
  kDropShadowDirection,
  // The content's drop shadow distance, the type is Float.
  kDropShadowDistance,
  // The content's drop shadow radius, the type is Float.
  kDropShadowRadius,
  // The enum end.
  kEnumEnd,
};

enum class ResourcePropertyType : uint16_t {
  kUnknown = 0,
  kImageDirName = kImageRangeStart,
  kImageFileName,
  kImageWidth,
  kImageHeight,

  kFontFamily = kFontRangeStart,
  kFontStyle,
  kFontAscent,
  kFontPath,

  kVideoDirName = kVideoRangeStart,
  kVideoFileName,
  kVideoWidth,
  kVideoHeight,

  kAudioDirName = kAudioRangeStart,
  kAudioFileName,

  kEnumEnd,
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_PROPERTY_TYPE_H_
