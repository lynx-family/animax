// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/text_gradient_skity_canvas_helper.h"

#include <memory>
#include <vector>

#include "skity/effect/mask_filter.hpp"
#include "skity/effect/shader.hpp"
#include "skity/graphic/paint.hpp"
#include "skity/text/font.hpp"
#include "skity/text/text_blob.hpp"
#include "skity/text/text_run.hpp"

namespace lynx {
namespace animax {
namespace {

std::shared_ptr<skity::TextBlob> BuildTextBlob(
    const ttoffice::tttext::ITypefaceHelper* font, uint32_t glyph_count,
    const uint16_t* glyphs, float* x, float* y,
    const ttoffice::tttext::Painter& painter, const skity::Paint& paint) {
  const auto* typeface =
      reinterpret_cast<const skity::textlayout::SkityTypefaceHelper*>(font);

  std::vector<skity::GlyphID> glyph_vector;
  std::vector<float> position_x;
  std::vector<float> position_y;
  glyph_vector.reserve(glyph_count);
  position_x.reserve(glyph_count);
  position_y.reserve(glyph_count);
  for (uint32_t index = 0; index < glyph_count; index++) {
    glyph_vector.push_back(glyphs[index]);
    position_x.push_back(x[index]);
    position_y.push_back(-y[index]);
  }

  std::vector<skity::TextRun> runs;
  skity::Font skity_font(typeface->GetTypeface(), paint.GetTextSize());
  skity_font.SetHinting(skity::Font::FontHinting::kSlight);
  skity_font.SetEmbolden(painter.IsBold());
  skity_font.SetSkewX(
      (painter.IsItalic() ? ttoffice::tttext::FAKE_ITALIC_SKEW : 0.f) +
      painter.GetTextSkew());
  runs.emplace_back(skity_font, glyph_vector, position_x, position_y);
  return std::make_shared<skity::TextBlob>(runs);
}

bool HasColorGlyphs(const ttoffice::tttext::ITypefaceHelper* font,
                    uint32_t glyph_count, const uint16_t* glyphs,
                    const skity::Paint& paint) {
  const auto* typeface_helper =
      reinterpret_cast<const skity::textlayout::SkityTypefaceHelper*>(font);
  const auto& typeface = typeface_helper->GetTypeface();
  if (typeface->ContainsColorTable()) {
    return true;
  }
  if (glyph_count == 0 || glyphs == nullptr) {
    return false;
  }

  skity::Font skity_font(typeface, paint.GetTextSize());
  std::vector<const skity::GlyphData*> glyph_data(glyph_count);
  skity_font.LoadGlyphMetrics(glyphs, glyph_count, glyph_data.data(), paint);
  for (const auto* glyph : glyph_data) {
    if (!glyph) {
      continue;
    }
    const auto format = glyph->GetFormat();
    if (format && *format != skity::GlyphFormat::A8) {
      return true;
    }
  }
  return false;
}

void DrawTextBlobPass(skity::Canvas* canvas,
                      const std::shared_ptr<skity::TextBlob>& text_blob,
                      float origin_x, float origin_y, skity::Paint paint,
                      const ttoffice::tttext::Painter& painter,
                      bool copy_text_shadows) {
  const auto& shadow_list = painter.GetShadowList();
  if (copy_text_shadows && !shadow_list.empty()) {
    skity::Paint shadow_paint = paint;
    for (const auto& shadow : shadow_list) {
      shadow_paint.SetColor(shadow.color_);
      if (shadow.blur_radius_ != 0) {
        std::shared_ptr<skity::MaskFilter> filter = skity::MaskFilter::MakeBlur(
            skity::BlurStyle::kNormal, shadow.blur_radius_);
        shadow_paint.SetMaskFilter(filter);
      }
      canvas->DrawTextBlob(text_blob, origin_x + shadow.offset_[0],
                           origin_y + shadow.offset_[1], shadow_paint);
    }
  }

  canvas->DrawTextBlob(text_blob, origin_x, origin_y, paint);
}

}  // namespace

TextGradientSkityCanvasHelper::TextGradientSkityCanvasHelper(
    skity::Canvas* canvas,
    const std::vector<std::shared_ptr<skity::Shader>>& shaders,
    uint8_t gradient_alpha)
    : SkityCanvasHelper(canvas),
      canvas_(canvas),
      gradient_alpha_(gradient_alpha) {
  shaders_.reserve(shaders.size());
  for (const auto& shader : shaders) {
    if (shader) {
      shaders_.push_back(shader);
    }
  }
}

void TextGradientSkityCanvasHelper::DrawGlyphs(
    const ttoffice::tttext::ITypefaceHelper* font, uint32_t glyph_count,
    const uint16_t* glyphs, const char* text, uint32_t text_bytes,
    float origin_x, float origin_y, float* x, float* y,
    ttoffice::tttext::Painter* painter) {
  skity::Paint source_paint = *ToSkityPaint(painter);
  const auto source_style = source_paint.GetStyle();
  const bool supports_gradient =
      source_style == skity::Paint::kFill_Style ||
      source_style == skity::Paint::kStrokeAndFill_Style ||
      source_style == skity::Paint::kStrokeThenFill_Style;
  if (shaders_.empty() || !supports_gradient) {
    SkityCanvasHelper::DrawGlyphs(font, glyph_count, glyphs, text, text_bytes,
                                  origin_x, origin_y, x, y, painter);
    return;
  }

  // Color glyphs, including emoji, are rendered from RGBA/BGRA data and do
  // not consume the text shader. Keep the original single pass so multi-pass
  // gradient rendering does not draw the color glyph repeatedly.
  if (HasColorGlyphs(font, glyph_count, glyphs, source_paint)) {
    SkityCanvasHelper::DrawGlyphs(font, glyph_count, glyphs, text, text_bytes,
                                  origin_x, origin_y, x, y, painter);
    return;
  }

  // TextBlob geometry is immutable across material passes; only Paint changes.
  auto text_blob =
      BuildTextBlob(font, glyph_count, glyphs, x, y, *painter, source_paint);

  skity::Paint base_paint(source_paint);
  base_paint.SetShader(nullptr);
  // StrokeAndFill needs separate fill and stroke draws: gradient shaders
  // replace only the fill, while the original solid stroke is drawn last.
  // StrokeThenFill can keep its combined order in the base draw.
  if (source_style == skity::Paint::kStrokeAndFill_Style) {
    base_paint.SetStyle(skity::Paint::kFill_Style);
  }
  DrawTextBlobPass(canvas_, text_blob, origin_x, origin_y, base_paint, *painter,
                   true);

  for (auto it = shaders_.rbegin(); it != shaders_.rend(); ++it) {
    skity::Paint gradient_paint(source_paint);
    gradient_paint.SetStyle(skity::Paint::kFill_Style);
    // Gradient stop alpha is local to the gradient. Keep the layer alpha as
    // the Paint's global alpha instead of inheriting the base fc alpha.
    gradient_paint.SetAlpha(gradient_alpha_);
    gradient_paint.SetShader(*it);
    DrawTextBlobPass(canvas_, text_blob, origin_x, origin_y, gradient_paint,
                     *painter, false);
  }

  if (source_style == skity::Paint::kStrokeAndFill_Style) {
    skity::Paint stroke_paint(source_paint);
    stroke_paint.SetStyle(skity::Paint::kStroke_Style);
    stroke_paint.SetShader(nullptr);
    DrawTextBlobPass(canvas_, text_blob, origin_x, origin_y, stroke_paint,
                     *painter, false);
  }
}

}  // namespace animax
}  // namespace lynx
