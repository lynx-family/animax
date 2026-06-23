// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/text_content_textra.h"

#include "src/base/log/log.h"
#include "src/layer/textra/text_content_layout_context.h"
#include "src/model/value/document_data.h"

namespace lynx {
namespace animax {

TextContentTextra::TextContentTextra(const TextContentDataSource& data_source)
    : data_source_(data_source), paint_(std::make_unique<Paint>()) {}

TextContentTextra::~TextContentTextra() = default;

void TextContentTextra::Draw(Canvas& canvas, int32_t alpha) {
  if (!data_source_.GetLayoutOnlyOnce()) {
    layout_context_.reset();
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
    layout_context_->Layout(data_source_, paint_->GetPaint());
  }

  auto* layout_region = layout_context_->GetLayoutRegion();
  DCHECK(layout_region);

  if (!layout_context_->IsBoxMode()) {
    const auto width = tttext::LAYOUT_MAX_UNITS;
    switch (document_data.GetJustification()) {
      case DocumentJustification::kRightAlign:
        canvas.Translate(-width, 0);
        break;
      case DocumentJustification::kCenter:
        canvas.Translate(-width / 2, 0);
        break;
      default:
        break;
    }
    auto* first_line = layout_region->GetLine(0);
    if (first_line) {
      canvas.Translate(0, -first_line->GetMaxAscent());
    }
  }

  auto* skity_canvas = canvas.GetSkityCanvas();
  ttoffice::tttext::SkityCanvasHelper canvas_helper(skity_canvas);
  ttoffice::tttext::LayoutDrawer drawer(&canvas_helper);
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
  auto baseline_shift = document_data.GetBaselineShift();
  if (baseline_shift != 0) {
    y -= baseline_shift;
  }

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
