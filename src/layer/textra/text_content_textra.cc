// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/text_content_textra.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "src/base/log/log.h"
#include "src/layer/text_gradient_helper.h"
#include "src/layer/textra/text_content_layout_context.h"
#include "src/layer/textra/text_gradient_skity_canvas_helper.h"
#include "src/model/value/document_data.h"

namespace lynx {
namespace animax {

TextContentTextra::TextContentTextra(const TextContentDataSource& data_source)
    : data_source_(data_source), paint_(std::make_unique<Paint>()) {}

TextContentTextra::~TextContentTextra() = default;

void TextContentTextra::Draw(Canvas& canvas, int32_t alpha) {
  if (!data_source_.GetLayoutOnlyOnce()) {
    layout_context_.reset();
    gradient_shaders_.clear();
    gradient_shaders_initialized_ = false;
  }

  // Offset by the box position
  auto& document_data = data_source_.GetDocumentData();
  auto* box_position = document_data.GetBoxPosition();
  if (box_position && !box_position->IsEmpty()) {
    canvas.Translate(box_position->GetX(), box_position->GetY());
  }

  // The baseline shift impacts the entire text for now.
  // Therefore, we use the translateY to achieve the same effect.
  auto baseline_shift = document_data.GetBaselineShift();
  if (baseline_shift != 0) {
    canvas.Translate(0, -baseline_shift);
  }

  if (layout_context_ == nullptr) {
    ConfigurePlatformPainter(data_source_, alpha);
    layout_context_ = std::make_unique<TextContentLayoutContext>();
    layout_context_->Layout(data_source_, paint_->GetPaint(), alpha);
  }

  auto* layout_region = layout_context_->GetLayoutRegion();
  DCHECK(layout_region);

  float origin_left = 0.f;
  float origin_width = layout_region->GetLayoutedWidth();
  float origin_height = layout_region->GetLayoutedHeight();
  if (layout_context_->IsBoxMode()) {
    auto* box_size = document_data.GetBoxSize();
    if (box_size && !box_size->IsEmpty()) {
      origin_width = box_size->GetX();
      origin_height = box_size->GetY();
    }
  } else {
    const auto width = tttext::LAYOUT_MAX_UNITS;
    switch (document_data.GetJustification()) {
      case DocumentJustification::kRightAlign:
        canvas.Translate(-width, 0);
        origin_left = width - origin_width;
        break;
      case DocumentJustification::kCenter:
        canvas.Translate(-width / 2, 0);
        origin_left = (width - origin_width) / 2;
        break;
      default:
        break;
    }
    auto* first_line = layout_region->GetLine(0);
    if (first_line) {
      canvas.Translate(0, -first_line->GetMaxAscent());
    }
  }

  const auto* text_gradient_model =
      layer_ ? layer_->GetLayerModel().GetTextGradientModel() : nullptr;
  const bool has_gradient = text_gradient_model != nullptr &&
                            !text_gradient_model->GetItems().empty();

  // A SaveLayer is required to fully preserve complex opacity composition
  // across multiple translucent gradient passes. Avoid it here because Textra
  // bounds can be inaccurate for some exported fonts and synthetic italic or
  // bold styles, and an offscreen layer adds memory and compositing overhead.
  std::unique_ptr<ttoffice::tttext::SkityCanvasHelper> canvas_helper;
  if (has_gradient) {
    if (!gradient_shaders_initialized_) {
      gradient_shaders_.clear();
      for (const auto& item : text_gradient_model->GetItems()) {
        auto shader = MakeTextGradientShader(*item, origin_width, origin_height,
                                             origin_left, 0.f);
        if (shader && shader->GetShader()) {
          gradient_shaders_.push_back(shader->GetShader());
        }
      }
      gradient_shaders_initialized_ = true;
    }

    canvas_helper = std::make_unique<TextGradientSkityCanvasHelper>(
        canvas.GetSkityCanvas(), gradient_shaders_,
        static_cast<uint8_t>(std::clamp(alpha, 0, 255)));
  } else {
    canvas_helper = std::make_unique<ttoffice::tttext::SkityCanvasHelper>(
        canvas.GetSkityCanvas());
  }

  ttoffice::tttext::LayoutDrawer drawer(canvas_helper.get());
  drawer.DrawLayoutPage(layout_region);
}

bool TextContentTextra::GetRect(RectF& out_rect) {
  if (!layout_context_) {
    return false;
  }

  auto& document_data = data_source_.GetDocumentData();
  double x = 0, y = 0, w = 0, h = 0;
  if (layout_context_->IsBoxMode()) {
    auto* box_position = document_data.GetBoxPosition();
    if (box_position && !box_position->IsEmpty()) {
      x = box_position->GetX();
      y = box_position->GetY();
    }
    auto* box_size = document_data.GetBoxSize();
    if (box_size && !box_size->IsEmpty()) {
      w = box_size->GetX();
      h = box_size->GetY();
    }
  } else {
    auto* layout_region = layout_context_->GetLayoutRegion();
    DCHECK(layout_region);
    w = layout_region->GetLayoutedWidth();
    h = layout_region->GetLayoutedHeight();
    switch (document_data.GetJustification()) {
      case DocumentJustification::kRightAlign:
        x = -w;
        break;
      case DocumentJustification::kCenter:
        x = -w / 2;
        break;
      default:
        break;
    }
    auto* first_line = layout_region->GetLine(0);
    if (first_line) {
      y = -first_line->GetMaxAscent();
    }
  }
  y -= document_data.GetBaselineShift();

  out_rect.Set(x, y, x + w, y + h);
  return true;
}

void TextContentTextra::ConfigurePlatformPainter(
    const TextContentDataSource& data_source, int32_t alpha) {
  // anti-aliasing default to true
  paint_->SetAntiAlias(true);

  auto& document_data = data_source.GetDocumentData();
  // fill color
  Color fill_color = Color(document_data.GetColor());
  fill_color.SetA(alpha * fill_color.GetA() / 255);
  paint_->SetFillColor(fill_color);

  float width = document_data.GetStrokeWidth();
  // stroke color and stroke width
  if (width > 0) {
    Color stroke_color = Color(document_data.GetStrokeColor());
    stroke_color.SetA(alpha * stroke_color.GetA() / 255);
    paint_->SetStrokeColor(stroke_color);
    paint_->SetStrokeWidth(width);
    // stroke overfill
    paint_->SetStyle(document_data.GetStrokeOverfill()
                         ? PaintStyle::kFillAddStroke
                         : PaintStyle::kStrokeThenFill);
  } else {
    paint_->SetStyle(PaintStyle::kFill);
  }

  if (layer_) {
    layer_->ApplyEffects(*paint_);
  }
}

}  // namespace animax
}  // namespace lynx
