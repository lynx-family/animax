// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/text_content_layout_context.h"

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
}  // namespace

void TextContentLayoutContext::Layout(const TextContentDataSource& data_source,
                                      const skity::Paint& paint) {
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

  // Add Text Run
  paragraph_->AddTextRun(&style, document_data.GetText().c_str());

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
