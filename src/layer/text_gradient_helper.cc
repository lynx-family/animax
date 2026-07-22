// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/text_gradient_helper.h"

#include <cmath>
#include <cstdint>

#include "src/render/matrix.h"

namespace lynx {
namespace animax {
namespace {

enum class TextGradientGeometryType : uint8_t {
  kLinear = 0,
  kRadial,
  kEllipticalRadial,
  kConic,
};

// Gradient geometry resolved from whole-text normalized coordinates using the
// text rectangle's width and height. The local coordinate origin is (0, 0).
struct TextGradientGeometry {
  TextGradientGeometryType type = TextGradientGeometryType::kLinear;
  PointF start_point;
  PointF end_point;
  float radius_x = 0.f;
  float radius_y = 0.f;
  float start_angle = 0.f;
};

constexpr float kMinGradientExtent = 1e-3f;

bool IsFinitePoint(const PointF& point) {
  return std::isfinite(point.GetX()) && std::isfinite(point.GetY());
}

PointF ResolvePoint(const PointF& normalized_point, float bounds_width,
                    float bounds_height, float bounds_origin_x,
                    float bounds_origin_y) {
  return PointF(bounds_origin_x + normalized_point.GetX() * bounds_width,
                bounds_origin_y + normalized_point.GetY() * bounds_height);
}

bool HasValidGradientColor(const GradientColor& gradient_color) {
  return gradient_color.GetSize() >= 2 &&
         gradient_color.GetColors() != nullptr &&
         gradient_color.GetPositions() != nullptr;
}

bool ResolveTextGradientGeometry(const TextGradientItemModel& item,
                                 float bounds_width, float bounds_height,
                                 TextGradientGeometry& out_geometry,
                                 float bounds_origin_x, float bounds_origin_y) {
  if (!std::isfinite(bounds_width) || !std::isfinite(bounds_height) ||
      bounds_width <= kMinGradientExtent ||
      bounds_height <= kMinGradientExtent ||
      !IsFinitePoint(item.GetStartPoint()) ||
      !HasValidGradientColor(item.GetGradientColor())) {
    return false;
  }

  out_geometry.start_point =
      ResolvePoint(item.GetStartPoint(), bounds_width, bounds_height,
                   bounds_origin_x, bounds_origin_y);
  if (!IsFinitePoint(out_geometry.start_point)) {
    return false;
  }

  switch (item.GetType()) {
    case GradientType::kLinear: {
      const auto* end_point = item.GetEndPoint();
      if (end_point == nullptr || !IsFinitePoint(*end_point)) {
        return false;
      }
      out_geometry.type = TextGradientGeometryType::kLinear;
      out_geometry.end_point =
          ResolvePoint(*end_point, bounds_width, bounds_height, bounds_origin_x,
                       bounds_origin_y);
      if (!IsFinitePoint(out_geometry.end_point)) {
        return false;
      }
      const auto dx =
          out_geometry.end_point.GetX() - out_geometry.start_point.GetX();
      const auto dy =
          out_geometry.end_point.GetY() - out_geometry.start_point.GetY();
      const auto extent = std::hypot(dx, dy);
      return std::isfinite(extent) && extent > kMinGradientExtent;
    }
    case GradientType::kRadial: {
      const auto* radius_xy = item.GetRadiusXY();
      if (radius_xy != nullptr) {
        if (!IsFinitePoint(*radius_xy)) {
          return false;
        }
        out_geometry.type = TextGradientGeometryType::kEllipticalRadial;
        out_geometry.radius_x = radius_xy->GetX() * bounds_width;
        out_geometry.radius_y = radius_xy->GetY() * bounds_height;
        return std::isfinite(out_geometry.radius_x) &&
               std::isfinite(out_geometry.radius_y) &&
               out_geometry.radius_x > kMinGradientExtent &&
               out_geometry.radius_y > kMinGradientExtent;
      }

      const auto* end_point = item.GetEndPoint();
      if (end_point == nullptr || !IsFinitePoint(*end_point)) {
        return false;
      }
      out_geometry.type = TextGradientGeometryType::kRadial;
      out_geometry.end_point =
          ResolvePoint(*end_point, bounds_width, bounds_height, bounds_origin_x,
                       bounds_origin_y);
      if (!IsFinitePoint(out_geometry.end_point)) {
        return false;
      }
      const auto dx =
          out_geometry.end_point.GetX() - out_geometry.start_point.GetX();
      const auto dy =
          out_geometry.end_point.GetY() - out_geometry.start_point.GetY();
      out_geometry.radius_x = std::hypot(dx, dy);
      out_geometry.radius_y = out_geometry.radius_x;
      return std::isfinite(out_geometry.radius_x) &&
             out_geometry.radius_x > kMinGradientExtent;
    }
    case GradientType::kConic:
      out_geometry.type = TextGradientGeometryType::kConic;
      out_geometry.start_angle = item.GetStartAngle().value_or(0.f);
      return std::isfinite(out_geometry.start_angle);
    default:
      return false;
  }
}

}  // namespace

std::unique_ptr<Shader> MakeTextGradientShader(
    const TextGradientItemModel& item, float bounds_width, float bounds_height,
    float bounds_origin_x, float bounds_origin_y) {
  TextGradientGeometry geometry;
  if (!ResolveTextGradientGeometry(item, bounds_width, bounds_height, geometry,
                                   bounds_origin_x, bounds_origin_y)) {
    return nullptr;
  }

  const auto& gradient_color = item.GetGradientColor();
  Matrix local_matrix;
  std::unique_ptr<Shader> shader;
  switch (geometry.type) {
    case TextGradientGeometryType::kLinear:
      shader = Shader::MakeLinear(
          geometry.start_point, geometry.end_point, gradient_color.GetSize(),
          gradient_color.GetColors(), gradient_color.GetPositions(),
          ShaderTileMode::kClamp, local_matrix);
      break;
    case TextGradientGeometryType::kRadial:
      shader = Shader::MakeRadial(
          geometry.start_point, geometry.radius_x, gradient_color.GetSize(),
          gradient_color.GetColors(), gradient_color.GetPositions(),
          ShaderTileMode::kClamp, local_matrix);
      break;
    case TextGradientGeometryType::kEllipticalRadial:
      // A unit circular radial gradient becomes an ellipse after applying
      // T * S: (u, v) -> (start_x + radius_x * u, start_y + radius_y * v).
      // Skity samples through the inverse local matrix, so it subtracts
      // start_point before dividing by radius_x/radius_y. Its PreConcat stores
      // current * new; starting from identity, PreTranslate followed by
      // PreScale therefore produces T * S. Reversing the calls would produce
      // S * T and scale the sampling center as well.
      local_matrix.PreTranslate(geometry.start_point.GetX(),
                                geometry.start_point.GetY(), 0.f);
      local_matrix.PreScale(geometry.radius_x, geometry.radius_y, 1.f);
      shader = Shader::MakeRadial(
          PointF(0.f, 0.f), 1.f, gradient_color.GetSize(),
          gradient_color.GetColors(), gradient_color.GetPositions(),
          ShaderTileMode::kClamp, local_matrix);
      break;
    case TextGradientGeometryType::kConic: {
      // Skity's sweep shader uses a mathematical angle whose zero boundary is
      // at the positive x-axis and increases clockwise in canvas coordinates.
      // CSS conic-gradient starts at 12 o'clock and uses a clockwise angle, so
      // a 270-degree offset maps CSS `from 0deg` to the top of the text box.
      // The full turn is cyclic, hence Repeat rather than Clamp.
      constexpr float kCssConicAngleOffset = 270.f;
      shader = Shader::MakeSweep(
          geometry.start_point, geometry.start_angle + kCssConicAngleOffset,
          geometry.start_angle + kCssConicAngleOffset + 360.f,
          gradient_color.GetSize(), gradient_color.GetColors(),
          gradient_color.GetPositions(), ShaderTileMode::kRepeat, local_matrix);
      break;
    }
    default:
      return nullptr;
  }

  if (!shader || !shader->GetShader()) {
    return nullptr;
  }
  return shader;
}

}  // namespace animax
}  // namespace lynx
