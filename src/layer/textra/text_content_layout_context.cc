// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/text_content_layout_context.h"

#include <algorithm>

#include "base/include/string/string_utils.h"
#include "src/content/path/path_util.h"
#include "src/layer/text_content_data_source.h"
#include "src/layer/text_layer.h"
#include "src/model/value/document_data.h"
#include "src/resource/asset/font_asset.h"

namespace lynx {
namespace animax {
namespace {
using TTHorizontalAlignment = ttoffice::tttext::ParagraphHorizontalAlignment;
TTHorizontalAlignment ToHorizontalAlignment(
    DocumentJustification justification) {
  switch (justification) {
    case DocumentJustification::kLeftAlign:
      return TTHorizontalAlignment::kLeft;
    case DocumentJustification::kRightAlign:
      return TTHorizontalAlignment::kRight;
    case DocumentJustification::kCenter:
      return TTHorizontalAlignment::kCenter;
    case DocumentJustification::kJustifyWithLastLineLeft:
    case DocumentJustification::kJustifyWithLastLineRight:
    case DocumentJustification::kJustifyWithLastLineCenter:
    case DocumentJustification::kJustifyWithLastLineFull:
      return TTHorizontalAlignment::kJustify;
    default:
      return TTHorizontalAlignment::kLeft;
  }
}

bool ConfigureRangeStyle(
    const TextContentDataSource::RangeStyle& range_style,
    const DocumentData& document_data, const skity::Paint& paint,
    int32_t layer_alpha, ttoffice::tttext::Style& style_for_range,
    std::vector<std::unique_ptr<ttoffice::tttext::SkityPainter>>&
        range_painters) {
  if (!range_style.HasStyle()) {
    return false;
  }

  if (range_style.skew != 0.f) {
    style_for_range.SetTextSkew(
        -std::tan(PathUtil::ToRadians(range_style.skew)));
  }

  if (range_style.color.IsEmpty() && range_style.stroke_color.IsEmpty() &&
      range_style.stroke_width == 0.f) {
    // Skew-only ranges still need ApplyStyleInRange, but do not need a painter.
    return true;
  }

  auto range_painter = std::make_unique<ttoffice::tttext::SkityPainter>();
  auto range_paint = std::make_unique<skity::Paint>(paint);
  auto layer_alpha_factor = std::clamp(layer_alpha / 255.f, 0.f, 1.f);
  const bool range_has_fill = !range_style.color.IsEmpty();
  const bool range_has_stroke =
      !range_style.stroke_color.IsEmpty() || range_style.stroke_width != 0.f;
  if (range_has_fill) {
    auto alpha =
        std::clamp(range_style.color.GetA() * layer_alpha_factor, 0.f, 255.f);
    range_paint->SetFillColor(skity::ColorSetARGB(
        alpha, range_style.color.GetR(), range_style.color.GetG(),
        range_style.color.GetB()));
  }

  if (range_has_stroke) {
    auto stroke_color = !range_style.stroke_color.IsEmpty()
                            ? range_style.stroke_color
                            : Color(document_data.GetStrokeColor());
    auto stroke_width = range_style.stroke_width != 0.f
                            ? range_style.stroke_width
                            : document_data.GetStrokeWidth();
    auto stroke_alpha =
        std::clamp(stroke_color.GetA() * layer_alpha_factor, 0.f, 255.f);
    range_paint->SetStrokeColor(
        skity::ColorSetARGB(stroke_alpha, stroke_color.GetR(),
                            stroke_color.GetG(), stroke_color.GetB()));
    range_paint->SetStrokeWidth(stroke_width);
    style_for_range.SetTextStrokeStyle(
        ttoffice::tttext::TTColor(stroke_color.GetInt()), stroke_width);
  }

  const auto base_style = range_paint->GetStyle();
  const bool base_has_fill = base_style != skity::Paint::kStroke_Style;
  const bool base_has_stroke = base_style != skity::Paint::kFill_Style;
  const bool should_draw_fill = base_has_fill || range_has_fill;
  const bool should_draw_stroke = base_has_stroke || range_has_stroke;
  if (should_draw_fill && should_draw_stroke) {
    range_paint->SetStyle(document_data.GetStrokeOverfill()
                              ? skity::Paint::kStrokeAndFill_Style
                              : skity::Paint::kStrokeThenFill_Style);
  } else if (should_draw_stroke) {
    range_paint->SetStyle(skity::Paint::kStroke_Style);
  } else {
    range_paint->SetStyle(skity::Paint::kFill_Style);
  }

  range_painter->platform_painter_ = std::move(range_paint);
  style_for_range.SetForegroundPainter(range_painter.get());
  range_painters.push_back(std::move(range_painter));
  return true;
}
}  // namespace

void TextContentLayoutContext::Layout(const TextContentDataSource& data_source,
                                      const skity::Paint& paint,
                                      int32_t layer_alpha) {
  auto& document_data = data_source.GetDocumentData();
  auto* box_size = document_data.GetBoxSize();
  const auto box_width =
      (box_size && !box_size->IsEmpty()) ? box_size->GetX() : 0.f;
  const auto box_height =
      (box_size && !box_size->IsEmpty()) ? box_size->GetY() : 0.f;
  is_box_mode_ = box_width > 0 && box_height > 0;

  // Create paragraph
  ttoffice::tttext::ParagraphStyle paragraph_style;
  paragraph_ = TTParagraph::Create();
  paragraph_style.SetHorizontalAlign(
      ToHorizontalAlignment(document_data.GetJustification()));
  paragraph_style.SetLineHeightInPx(document_data.GetLineHeight(),
                                    ttoffice::tttext::RulerType::kExact);
  paragraph_style.SetVerticalAlign(
      ttoffice::tttext::ParagraphVerticalAlignment::kTop);
  paragraph_style.SetLineHeightOverride(true);
  paragraph_->SetParagraphStyle(&paragraph_style);

  // Text size
  float text_size = data_source.GetTextSize();

  // Set tracking
  constexpr float size_scale = 100.f;
  float tracking = data_source.GetTracking() * text_size / size_scale;

  ttoffice::tttext::Style style;
  style.SetLetterSpacing(tracking);
  // Set font descriptor
  ttoffice::tttext::FontDescriptor font_descriptor;
  font_descriptor.font_family_list_.clear();

  font_descriptor.font_family_list_.push_back(data_source.GetFontName());

  auto* font_asset = data_source.GetFontAsset();
  if (font_asset && font_asset->GetFont()) {
    // enum values are all same, see skity/text/font_style.hpp and
    // textra/font_info.h
    auto& style = font_asset->GetFont()->GetFontStyle();
    font_descriptor.font_style_.SetWidth(style.width());
    font_descriptor.font_style_.SetWeight(style.weight());
    font_descriptor.font_style_.SetSlant(style.slant());
  }

  style.SetFontDescriptor(font_descriptor);

  // Set text size
  style.SetTextSize(text_size);

  // Config TT_Painter
  tt_painter_.platform_painter_ = std::make_unique<skity::Paint>(paint);
  style.SetForegroundPainter(&tt_painter_);
  range_painters_.clear();

  // Add Text Run
  paragraph_->AddTextRun(&style, document_data.GetText().c_str());
  const size_t text_length = paragraph_->GetCharCount();

  // Add Text Range Selector
  auto range_styles = data_source.GetRangeAnimatorPropertyList(text_length);
  for (const auto& range_style : range_styles) {
    ttoffice::tttext::Style style_for_range;
    if (!ConfigureRangeStyle(range_style, document_data, paint, layer_alpha,
                             style_for_range, range_painters_)) {
      continue;
    }
    paragraph_->ApplyStyleInRange(
        style_for_range, range_style.segment_start,
        range_style.segment_end - range_style.segment_start);
  }

  // Layout
  ttoffice::tttext::TTTextContext context;
  ttoffice::tttext::TextLayout text_layout(
      static_cast<TTFontMgrCollection*>(data_source.GetFontMgrCollection()),
      ttoffice::tttext::ShaperType::kSelfRendering);
  if (is_box_mode_) {
    // Layout by text box
    layout_region_ =
        std::make_unique<ttoffice::tttext::LayoutRegion>(box_width, box_height);
  } else {
    // Layout one line with at most width mode
    layout_region_ = std::make_unique<ttoffice::tttext::LayoutRegion>(
        ttoffice::tttext::LAYOUT_MAX_UNITS, ttoffice::tttext::LAYOUT_MAX_UNITS,
        ttoffice::tttext::LayoutMode::kAtMost,
        ttoffice::tttext::LayoutMode::kAtMost);
  }
  text_layout.Layout(paragraph_.get(), layout_region_.get(), context);
}

}  // namespace animax
}  // namespace lynx
