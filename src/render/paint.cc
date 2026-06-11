// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/paint.h"

#include <algorithm>
#include <cmath>

#include "skity/effect/color_filter.hpp"
#include "skity/effect/image_filter.hpp"
#include "skity/graphic/paint.hpp"
#include "src/render/mask_filter.h"
#include "src/render/shader.h"

namespace lynx {
namespace animax {

Paint::Paint() : paint_(std::make_unique<skity::Paint>()) {}
Paint::~Paint() = default;

const skity::Paint &Paint::GetPaint() const { return *paint_; }

void Paint::SetAntiAlias(bool anti_alias) { paint_->SetAntiAlias(anti_alias); }

void Paint::SetAlpha(float alpha) {
  paint_->SetAlpha(static_cast<uint8_t>(std::clamp(alpha, 0.0f, 255.0f)));
}

void Paint::SetColor(const Color &color) {
  paint_->SetColor(skity::ColorSetARGB(color.GetA(), color.GetR(), color.GetG(),
                                       color.GetB()));
}

void Paint::SetColorFilter(ColorFilter &filter) {
  if (filter.IsEmpty()) {
    return;
  }
  auto skity_color_filter = skity::ColorFilters::Blend(
      filter.GetColor(), static_cast<skity::BlendMode>(filter.GetMode()));
  paint_->SetColorFilter(skity_color_filter);
}

void Paint::SetStyle(PaintStyle style) {
  switch (style) {
    case PaintStyle::kFill:
      paint_->SetStyle(skity::Paint::kFill_Style);
      break;
    case PaintStyle::kStroke:
      paint_->SetStyle(skity::Paint::kStroke_Style);
      break;
    case PaintStyle::kFillAddStroke:
      paint_->SetStyle(skity::Paint::kStrokeAndFill_Style);
      break;
    case PaintStyle::kStrokeThenFill:
      paint_->SetStyle(skity::Paint::kStrokeThenFill_Style);
      break;
    default:
      paint_->SetStyle(skity::Paint::kFill_Style);
      break;
  }
}

void Paint::SetFillColor(const Color &color) {
  paint_->SetFillColor(skity::ColorSetARGB(color.GetA(), color.GetR(),
                                           color.GetG(), color.GetB()));
}

void Paint::SetStrokeColor(const Color &color) {
  paint_->SetStrokeColor(skity::ColorSetARGB(color.GetA(), color.GetR(),
                                             color.GetG(), color.GetB()));
}

void Paint::SetStrokeCap(PaintCap cap) {
  switch (cap) {
    case PaintCap::kButt:
      paint_->SetStrokeCap(skity::Paint::kButt_Cap);
      break;
    case PaintCap::kRound:
      paint_->SetStrokeCap(skity::Paint::kRound_Cap);
      break;
    case PaintCap::kSquare:
      paint_->SetStrokeCap(skity::Paint::kSquare_Cap);
      break;
    default:
      paint_->SetStrokeCap(skity::Paint::kButt_Cap);
      break;
  }
}

void Paint::SetStrokeJoin(PaintJoin join) {
  switch (join) {
    case PaintJoin::kMiter:
      paint_->SetStrokeJoin(skity::Paint::kMiter_Join);
      break;
    case PaintJoin::kBevel:
      paint_->SetStrokeJoin(skity::Paint::kBevel_Join);
      break;
    case PaintJoin::kRound:
      paint_->SetStrokeJoin(skity::Paint::kRound_Join);
      break;
    default:
      paint_->SetStrokeJoin(skity::Paint::kRound_Join);
      break;
  }
}

void Paint::SetStrokeMiter(float miter) { paint_->SetStrokeMiter(miter); }

void Paint::SetStrokeWidth(float width) { paint_->SetStrokeWidth(width); }

void Paint::SetXfermode(PaintXfermode mode) {
  switch (mode) {
    case PaintXfermode::kDstOut:
      paint_->SetBlendMode(skity::BlendMode::kDstOut);
      break;
    case PaintXfermode::kDstIn:
      paint_->SetBlendMode(skity::BlendMode::kDstIn);
      break;
    case PaintXfermode::kClear:
      paint_->SetBlendMode(skity::BlendMode::kClear);
      break;
    default:
      paint_->SetBlendMode(skity::BlendMode::kDefault);
      break;
  }
}

void Paint::SetShader(Shader *shader) {
  if (shader == nullptr) {
    paint_->SetShader(nullptr);
    return;
  }

  paint_->SetShader(shader->GetShader());
}

void Paint::SetShadowLayer(float radius, float x, float y, int32_t color) {
  shadow_layer_ = std::unique_ptr<SkityShadowLayer>(
      new SkityShadowLayer(std::max(radius, 3.f), x, y, color));
}

void Paint::SetMaskFilter(MaskFilter *filter) {
  if (filter) {
    paint_->SetMaskFilter(filter->GetMaskFilter());
  } else {
    paint_->SetMaskFilter(nullptr);
  }
}

void Paint::SetDashPathEffect(DashPathEffect &effect) {
  paint_->SetPathEffect(effect.GetEffect());
}

void Paint::SetFontThreshold(float font_size) {
  paint_->SetFontThreshold(font_size);
}

float Paint::GetStrokeWidth() const { return paint_->GetStrokeWidth(); }
}  // namespace animax
}  // namespace lynx
