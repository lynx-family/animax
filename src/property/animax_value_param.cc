// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/property/animax_value_param.h"

#include "src/model/value/base_value.h"
#include "src/model/value/document_data.h"
#include "src/render/color_filter.h"

namespace lynx {
namespace animax {

namespace {
ValueType GetValueTypeForProperty(LayerPropertyType property_type) {
  switch (property_type) {
    case LayerPropertyType::kTransformRotation:
    case LayerPropertyType::kTransformSkew:
    case LayerPropertyType::kTransformSkewAngle:
    case LayerPropertyType::kTransformStartOpacity:
    case LayerPropertyType::kTransformEndOpacity:
    case LayerPropertyType::kTransformRotationX:
    case LayerPropertyType::kTransformRotationY:
    case LayerPropertyType::kTransformRotationZ:
    case LayerPropertyType::kTextSize:
    case LayerPropertyType::kTextTracking:
    case LayerPropertyType::kTextSkew:
    case LayerPropertyType::kTextRangeOffset:
    case LayerPropertyType::kTextRangeStart:
    case LayerPropertyType::kTextRangeEnd:
    case LayerPropertyType::kStrokeWidth:
    case LayerPropertyType::kBlurRadius:
    case LayerPropertyType::kCornerRadius:
    case LayerPropertyType::kRepeaterCopies:
    case LayerPropertyType::kPolyStarPoints:
    case LayerPropertyType::kPolyStarRotation:
    case LayerPropertyType::kPolyStarInnerRadius:
    case LayerPropertyType::kPolyStarOuterRadius:
    case LayerPropertyType::kPolyStarInnerRounded:
    case LayerPropertyType::kPolyStarOuterRounded:
    case LayerPropertyType::kDropShadowDirection:
    case LayerPropertyType::kDropShadowDistance:
    case LayerPropertyType::kDropShadowRadius:
      return ValueType::kFloat;

    case LayerPropertyType::kVisibility:
    case LayerPropertyType::kTransformOpacity:
    case LayerPropertyType::kOpacity:
    case LayerPropertyType::kDropShadowOpacity:
      return ValueType::kInteger;

    case LayerPropertyType::kTransformPosition:
    case LayerPropertyType::kTransformAnchor:
    case LayerPropertyType::kPosition:
    case LayerPropertyType::kEllipseSize:
    case LayerPropertyType::kRectangleSize:
    case LayerPropertyType::kRepeaterOffset:
      return ValueType::kPoint;

    case LayerPropertyType::kTransformScale:
      return ValueType::kScale;

    case LayerPropertyType::kColor:
    case LayerPropertyType::kTextColor:
    case LayerPropertyType::kStrokeColor:
    case LayerPropertyType::kDropShadowColor:
      return ValueType::kColor;

    case LayerPropertyType::kColorFilter:
      return ValueType::kColorFilter;

    case LayerPropertyType::kTextValue:
      return ValueType::kDocument;

    default:
      return ValueType::kUnknown;
  }
}
}  // namespace

AnimaXValueParam::AnimaXValueParam(const std::string& value,
                                   int32_t target_frame)
    : type_(Type::kString), string_value_(value), target_frame_(target_frame) {}

AnimaXValueParam::AnimaXValueParam(double value, ApplyMode apply_mode,
                                   int32_t target_frame)
    : type_(Type::kNumber),
      number_value_(value),
      apply_mode_(apply_mode),
      target_frame_(target_frame) {}

AnimaXValueParam::AnimaXValueParam(bool value, int32_t target_frame)
    : type_(Type::kBoolean),
      number_value_(value ? 1.0 : 0.0),
      target_frame_(target_frame) {}

AnimaXValueParam::AnimaXValueParam(double x, double y, ApplyMode apply_mode,
                                   int32_t target_frame)
    : type_(Type::kCoordinate),
      coordinate_value_(PointF(static_cast<float>(x), static_cast<float>(y))),
      apply_mode_(apply_mode),
      target_frame_(target_frame) {}

AnimaXValueParam::AnimaXValueParam(double x, double y, double z,
                                   ApplyMode apply_mode, int32_t target_frame)
    : type_(Type::kCoordinate),
      coordinate_value_(PointF(static_cast<float>(x), static_cast<float>(y),
                               static_cast<float>(z))),
      apply_mode_(apply_mode),
      target_frame_(target_frame) {}

AnimaXValueParam::AnimaXValueParam(int32_t color, int32_t target_frame)
    : type_(Type::kColor),
      color_value_(Color(color)),
      target_frame_(target_frame) {}

AnimaXValueParam::AnimaXValueParam(const ColorFilterParam& filter_params,
                                   int32_t target_frame)
    : type_(Type::kColorFilter),
      number_value_(static_cast<double>(filter_params.mode)),
      color_value_(Color(filter_params.color)),
      target_frame_(target_frame) {}

AnimaXValueParam::AnimaXValueParam()
    : type_(Type::kNull), target_frame_(kFrameIndexAll) {}

AnimaXValueParam& AnimaXValueParam::operator=(
    AnimaXValueParam&& other) noexcept {
  if (this != &other) {
    type_ = other.type_;
    string_value_ = std::move(other.string_value_);
    number_value_ = std::move(other.number_value_);
    coordinate_value_ = std::move(other.coordinate_value_);
    color_value_ = std::move(other.color_value_);
    apply_mode_ = other.apply_mode_;
    target_frame_ = other.target_frame_;
  }
  return *this;
}

const std::string& AnimaXValueParam::GetStringValue() const {
  if (string_value_.has_value()) {
    return *string_value_;
  }
  static const std::string kEmptyString = "";
  return kEmptyString;
}

double AnimaXValueParam::GetNumberValue() const {
  return number_value_.value_or(0.0);
}

bool AnimaXValueParam::GetBooleanValue() const {
  return number_value_.value_or(0.0) == 1.0;
}

double AnimaXValueParam::GetX() const {
  return coordinate_value_ ? coordinate_value_->GetX() : 0.0;
}

double AnimaXValueParam::GetY() const {
  return coordinate_value_ ? coordinate_value_->GetY() : 0.0;
}

double AnimaXValueParam::GetZ() const {
  return coordinate_value_ ? coordinate_value_->GetZ() : 0.0;
}

int32_t AnimaXValueParam::GetColorValue() const {
  if ((IsColor() || IsColorFilter()) && color_value_) {
    return color_value_->GetInt();
  }
  return 0;
}

void* AnimaXValueParam::GetValuePtr() const {
  switch (type_) {
    case Type::kString:
      return string_value_.has_value()
                 ? const_cast<std::string*>(&*string_value_)
                 : nullptr;
    case Type::kNumber:
    case Type::kBoolean:
      return number_value_.has_value() ? const_cast<double*>(&*number_value_)
                                       : nullptr;
    case Type::kCoordinate:
      return coordinate_value_.has_value()
                 ? const_cast<PointF*>(&*coordinate_value_)
                 : nullptr;
    case Type::kColor:
      return color_value_.has_value() ? const_cast<Color*>(&*color_value_)
                                      : nullptr;
    // Do not support kColorFilter
    case Type::kNull:
    default:
      return nullptr;
  }
}

std::unique_ptr<AnimaXValueParam> AnimaXValueParam::FromValue(
    const Value* value) {
  if (!value) {
    return std::make_unique<AnimaXValueParam>();
  }

  auto value_type = value->GetValueType();
  switch (value_type) {
    case ValueType::kFloat: {
      auto* float_value = static_cast<const Float*>(value);
      return std::make_unique<AnimaXValueParam>(float_value->Get(),
                                                ApplyMode::kSet);
    }

    case ValueType::kInteger: {
      auto* int_value = static_cast<const Integer*>(value);
      return std::make_unique<AnimaXValueParam>(
          static_cast<double>(int_value->Get()), ApplyMode::kSet);
    }

    case ValueType::kPoint: {
      auto* point_value = static_cast<const PointF*>(value);
      return std::make_unique<AnimaXValueParam>(
          point_value->GetX(), point_value->GetY(), point_value->GetZ(),
          ApplyMode::kSet);
    }

    case ValueType::kScale: {
      auto* scale_value = static_cast<const ScaleF*>(value);
      return std::make_unique<AnimaXValueParam>(
          scale_value->GetX(), scale_value->GetY(), scale_value->GetZ(),
          ApplyMode::kSet);
    }

    case ValueType::kColor: {
      auto* color_value = static_cast<const Color*>(value);
      return std::make_unique<AnimaXValueParam>(color_value->GetInt());
    }

    case ValueType::kColorFilter: {
      auto* color_filter_value = static_cast<const ColorFilter*>(value);
      return std::make_unique<AnimaXValueParam>(color_filter_value->GetColor(),
                                                color_filter_value->GetMode());
    }

    case ValueType::kDocument: {
      auto* document_value = static_cast<const DocumentData*>(value);
      return std::make_unique<AnimaXValueParam>(document_value->GetText());
    }

    case ValueType::kUnknown:
    default:
      return std::make_unique<AnimaXValueParam>();
  }
}

/**
 * Converts AnimaXValueParam to appropriate Value based on property type
 */
std::unique_ptr<Value> AnimaXValueParam::ToValue(
    LayerPropertyType property_type) const {
  ValueType value_type = GetValueTypeForProperty(property_type);
  if (value_type == ValueType::kUnknown) {
    return nullptr;
  }

  // Convert based on target value type
  switch (value_type) {
    case ValueType::kFloat:
      if (IsNumber()) {
        return ValueFactory::Make<Float>(static_cast<float>(GetNumberValue()));
      }
      break;

    case ValueType::kInteger:
      if (IsNumber()) {
        return ValueFactory::Make<Integer>(
            static_cast<int32_t>(GetNumberValue()));
      }
      break;

    case ValueType::kPoint:
      if (IsCoordinate()) {
        return ValueFactory::Make<PointF>(static_cast<float>(GetX()),
                                          static_cast<float>(GetY()),
                                          static_cast<float>(GetZ()));
      }
      break;

    case ValueType::kScale:
      if (IsCoordinate()) {
        return ValueFactory::Make<ScaleF>(static_cast<float>(GetX()),
                                          static_cast<float>(GetY()),
                                          static_cast<float>(GetZ()));
      }
      break;

    case ValueType::kColor:
      if (IsColor()) {
        return ValueFactory::Make<Color>(GetColorValue());
      }
      break;

    case ValueType::kColorFilter:
      if (IsColorFilter()) {
        return ValueFactory::Make<ColorFilter>(GetColorValue(),
                                               GetNumberValue());
      }
      break;

    case ValueType::kDocument:
      if (IsString()) {
        auto document_data = ValueFactory::Make<DocumentData>();
        document_data->SetText(GetStringValue());
        return document_data;
      }
      break;

    default:
      break;
  }

  return nullptr;
}

AnimaXValueParam::Type AnimaXValueParam::GetType() const { return type_; }

bool AnimaXValueParam::CopyTo(std::string* target) const {
  if (!target || !IsString() || !string_value_.has_value()) {
    return false;
  }
  *target = *string_value_;
  return true;
}

bool AnimaXValueParam::CopyTo(float* target) const {
  if (!target || !IsNumber() || !number_value_.has_value()) {
    return false;
  }
  *target = static_cast<float>(*number_value_);
  return true;
}

bool AnimaXValueParam::CopyTo(int32_t* target) const {
  if (!target || !IsNumber() || !number_value_.has_value()) {
    return false;
  }
  *target = static_cast<int32_t>(*number_value_);
  return true;
}

bool AnimaXValueParam::CopyTo(double* target) const {
  if (!target || !IsNumber() || !number_value_.has_value()) {
    return false;
  }
  *target = *number_value_;
  return true;
}

bool AnimaXValueParam::CopyTo(bool* target) const {
  if (!target || !IsBoolean() || !number_value_.has_value()) {
    return false;
  }
  *target = (*number_value_ == 1.0);
  return true;
}

}  // namespace animax
}  // namespace lynx
